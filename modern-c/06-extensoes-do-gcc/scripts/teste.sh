#!/usr/bin/env bash
# Testa os exemplos: compila e roda tudo, confere que os casos de erros/ falham pelo motivo
# certo, confere o efeito da barreira de compilador no assembly e, se houver um GCC para
# ARM, compila as instruções do Cortex-M.
#
# Uso: scripts/teste.sh            (CC=clang scripts/teste.sh)
#      ARM_CC=/caminho/arm-none-eabi-gcc scripts/teste.sh
set -uo pipefail

cd "$(dirname "$0")/.."
CC="${CC:-cc}"
falhas=0
ok()    { echo "  ok: $1"; }
falha() { echo "  FALHOU: $1"; falhas=$((falhas + 1)); }

echo "== exemplos (make run)"
log="$(mktemp)"
if make -s clean && make -s CC="$CC" run > "$log" 2>&1; then
    ok "todos compilam e rodam"
else
    cat "$log"; falha "make run"
fi
rm -f "$log"

# espera_falha <descrição> <padrão> <argumentos...>
espera_falha() {
    local desc="$1" padrao="$2"; shift 2
    local saida
    if saida="$("$CC" "$@" -o /dev/null 2>&1)"; then
        falha "$desc (compilou, mas devia falhar)"
    elif grep -q -- "$padrao" <<< "$saida"; then
        ok "$desc"
    else
        echo "$saida" | head -5; falha "$desc (falhou, mas sem \"$padrao\")"
    fi
}
espera_sucesso() {
    local desc="$1"; shift
    local saida
    if saida="$("$CC" "$@" -o /dev/null 2>&1)"; then ok "$desc"; else echo "$saida" | head -5; falha "$desc"; fi
}

echo "== casos que devem falhar (erros/)"
espera_falha   "format(printf): %d com string" "format" -std=gnu17 -Werror=format erros/format_errado.c
espera_falha   "packed: endereço de membro empacotado" "packed member" -std=gnu17 -Werror -c erros/packed_ponteiro.c
espera_falha   "nonnull: NULL em parâmetro nonnull" "nonnull" -std=gnu17 -Wall -Werror -c erros/nonnull.c
espera_falha   "warn_unused_result: retorno ignorado" "unused" -std=gnu17 -Werror -c erros/warn_unused_result.c
espera_falha   "-pedantic-errors: statement expression" "ISO C forbids braced-groups\|GNU statement expression" -std=gnu17 -pedantic-errors erros/pedantic.c
espera_falha   "-pedantic-errors: array de tamanho zero" "zero-size array\|zero size arrays" -std=gnu17 -pedantic-errors erros/tamanho_zero.c
espera_falha   "compat: PACKED sem suporte vira erro" "PACKED: defina" -std=gnu17 -DCOMPAT_SIMULA_OUTRO_COMPILADOR -c compat/usa_packed.c
espera_sucesso "compat: PACKED com suporte" -std=gnu17 -c compat/usa_packed.c

if "$CC" --version | grep -qi clang; then
    echo "  pulado: casos só do GCC (funções aninhadas, macro linux) no Clang"
else
    espera_falha   "função aninhada com -Werror=trampolines" "trampoline" -std=gnu17 -Werror=trampolines erros/aninhada.c
    if echo | "$CC" -std=gnu17 -dM -E -x c - | grep -q '#define linux 1'; then
        espera_sucesso "int linux = 1 em -std=c17" -std=c17 erros/linux_gnu17.c
        espera_falha   "int linux = 1 em -std=gnu17" "expected identifier" -std=gnu17 erros/linux_gnu17.c
    else
        echo "  pulado: a macro linux só existe no host Linux"
    fi
fi

# laco_infinito <compilador e flags...>: imprime as funções de asm/barreira.c que viraram
# um laço sobre o mesmo rótulo (".L3: jmp .L3" no x86, ".L3: b .L3" no ARM).
laco_infinito() {
    "$@" -std=gnu17 -O2 -S asm/barreira.c -o - | awk '
        /^[a-z_]+:/ { func = $1; sub(":", "", func) }
        /^\.L[0-9]+:/ { rot = $1; sub(":", "", rot); next }
        rot != "" && ($1 == "jmp" || $1 == "b") && $2 == rot { print func }
        { if ($1 !~ /^\./ && $1 !~ /^@/) rot = "" }'
}

echo "== barreira de compilador (asm/barreira.c, rodando de verdade)"
# Um sinal liga flag depois de 200 ms. "com" deve retornar com flag = 1. "sem" mostra o
# defeito: o GCC trava (laço infinito) e o Clang retorna na hora, com flag = 0.
tmp="$(mktemp -d)"
if "$CC" -std=gnu17 -O2 asm/barreira.c asm/espera_main.c -o "$tmp/espera" 2> "$tmp/log"; then
    com="$(timeout 3 "$tmp/espera" com || echo "travou")"
    sem="$(timeout 3 "$tmp/espera" sem || echo "travou")"
    echo "  com barreira: $com"
    echo "  sem barreira: $sem"
    if [ "$com" = "retornou com flag = 1" ] && [ "$sem" != "retornou com flag = 1" ]; then
        ok "a barreira faz a diferença"
    else
        falha "comportamento inesperado da barreira"
    fi
else
    cat "$tmp/log"; falha "compilar asm/espera_main.c"
fi
rm -rf "$tmp"

echo "== Cortex-M"
if [ -z "${ARM_CC:-}" ]; then
    ARM_CC="$(command -v arm-none-eabi-gcc || true)"
fi
if [ -z "$ARM_CC" ]; then
    ARM_CC="$(ls ~/zephyr-sdk-*/gnu/arm-zephyr-eabi/bin/arm-zephyr-eabi-gcc \
                  ~/zephyr-sdk-*/arm-zephyr-eabi/bin/arm-zephyr-eabi-gcc 2>/dev/null | head -1 || true)"
fi
if [ -z "$ARM_CC" ]; then
    echo "  pulado: nenhum GCC para ARM (defina ARM_CC)"
else
    echo "  usando $("$ARM_CC" --version | head -1)"
    asm="$("$ARM_CC" -std=gnu17 -O2 -mcpu=cortex-m0 -mthumb -S asm/cortex_m.c -o - 2>&1)"
    if grep -q 'cpsid' <<< "$asm" && grep -q 'wfi' <<< "$asm"; then
        ok "cortex-m0: cpsid i / cpsie i / wfi no código gerado"
    else
        echo "$asm" | head; falha "cortex-m0: instruções não encontradas"
    fi
    r="$(laco_infinito "$ARM_CC" -mcpu=cortex-m4 -mthumb | sort -u | tr '\n' ' ')"
    if [ "$r" = "espera_sem_barreira " ]; then
        ok "cortex-m4: sem barreira vira laço infinito; com barreira relê flag"
    else
        falha "cortex-m4: laços infinitos encontrados em: '$r'"
    fi
fi

make -s clean
echo
if [ "$falhas" -eq 0 ]; then echo "tudo certo"; else echo "$falhas falha(s)"; exit 1; fi
