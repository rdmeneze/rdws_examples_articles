#!/usr/bin/env bash
# Confere cada diagnóstico: o aviso (ou erro) aparece com a opção certa e não aparece sem ela.
# Se houver um GCC para ARM, confere também o custo do double no Cortex-M4.
#
# Uso: scripts/teste.sh            (CC=clang scripts/teste.sh)
#      ARM_CC=/caminho/arm-none-eabi-gcc scripts/teste.sh
set -uo pipefail

cd "$(dirname "$0")/.."
CC="${CC:-cc}"
falhas=0
ok()    { echo "  ok: $1"; }
falha() { echo "  FALHOU: $1"; falhas=$((falhas + 1)); }
eh_clang() { "$CC" --version | grep -qi clang; }

# Usa -std=c23 se o compilador aceitar; senão, c2x.
if echo 'int x;' | "$CC" -std=c23 -x c -fsyntax-only - 2>/dev/null; then C23=c23; else C23=c2x; fi

saida() { "$CC" "$@" -c -o /dev/null 2>&1; }

# com_e_sem <descrição> <arquivo> <padrão> <flags base> -- <flags extras>
com_e_sem() {
    local desc="$1" arq="$2" padrao="$3"; shift 3
    local base=() extra=()
    while [ "$1" != "--" ]; do base+=("$1"); shift; done; shift
    extra=("$@")
    if grep -q -- "$padrao" <<< "$(saida "${base[@]}" "$arq")"; then
        falha "$desc: o aviso já aparece sem ${extra[*]}"
    elif grep -q -- "$padrao" <<< "$(saida "${base[@]}" "${extra[@]}" "$arq")"; then
        ok "$desc: só com ${extra[*]}"
    else
        falha "$desc: nem com ${extra[*]}"
    fi
}

# espera <descrição> <padrão> <flags...> : o diagnóstico precisa aparecer
espera() {
    local desc="$1" padrao="$2"; shift 2
    if grep -q -- "$padrao" <<< "$(saida "$@")"; then ok "$desc"; else falha "$desc"; fi
}
# limpo <descrição> <flags...> : compila sem nenhum aviso
limpo() {
    local desc="$1"; shift
    local r; r="$(saida "$@")"
    if [ -z "$r" ]; then ok "$desc"; else echo "$r" | head -3; falha "$desc"; fi
}

W="-Wall -Wextra"
echo "== avisos que o -Wall -Wextra não liga"
com_e_sem "truncamento uint32_t -> uint16_t" avisos/conversao.c "Wconversion\|Wimplicit-int-conversion" -std=c17 $W -- -Wconversion
com_e_sem "float promovido a double" avisos/double_promotion.c "Wdouble-promotion" -std=c17 $W -- -Wdouble-promotion
com_e_sem "macro de configuração indefinida no #if" avisos/undef.c "Wundef" -std=c17 $W -- -Wundef
com_e_sem "valor de enum esquecido atrás de um default" avisos/switch_enum.c "Wswitch-enum" -std=c17 $W -- -Wswitch-enum
com_e_sem "variável local escondendo a global" avisos/shadow.c "Wshadow" -std=c17 $W -- -Wshadow
espera "comparação com e sem sinal já aparece com -Wextra" "Wsign-compare" -std=c17 $W avisos/conversao.c

echo "== atributos"
espera "[[nodiscard]] com mensagem" "verifique o c" -std=$C23 atributos/nodiscard.c
espera "[[deprecated]] com mensagem" "sensor_read_v2" -std=$C23 atributos/deprecated.c
if eh_clang; then
    # No Clang, o -Wimplicit-fallthrough não faz parte do -Wextra.
    com_e_sem "switch sem break e sem nada" atributos/fallthrough_nada.c "fallthrough" -std=$C23 $W -- -Wimplicit-fallthrough
else
    espera "switch sem break e sem nada: avisa com -Wextra" "fallthrough" -std=$C23 $W atributos/fallthrough_nada.c
fi
limpo  "[[fallthrough]]: sem aviso" -std=$C23 $W atributos/fallthrough_atributo.c
if eh_clang; then
    espera "/* fall through */: o Clang ignora o comentário e avisa" "fallthrough" -std=$C23 -Wimplicit-fallthrough atributos/fallthrough_comentario.c
else
    limpo  "/* fall through */: o GCC aceita no nível padrão (3)" -std=$C23 $W atributos/fallthrough_comentario.c
    espera "/* fall through */: o GCC avisa no nível 5" "fallthrough" -std=$C23 -Wimplicit-fallthrough=5 atributos/fallthrough_comentario.c
    tmp="$(mktemp -d)"
    "$CC" -std=$C23 -E atributos/fallthrough_comentario.c -o "$tmp/pp.c"
    espera "/* fall through */: o aviso volta se o código for pré-processado antes" "fallthrough" -std=$C23 $W "$tmp/pp.c"
    rm -rf "$tmp"
fi

echo "== static_assert e pré-processador"
limpo  "configuração válida compila" -std=$C23 $W static_assert/configuracao.c
espera "RB_TAMANHO=100 quebra o build" "potência de 2" -std=$C23 -DRB_TAMANHO=100 static_assert/configuracao.c
espera "sem UART_BAUD, o #error para o build" "defina UART_BAUD" -std=$C23 static_assert/preprocessador.c
limpo  "UART_BAUD=115200 compila" -std=$C23 -DUART_BAUD=115200 static_assert/preprocessador.c
espera "UART_BAUD alto gera o #warning" "confira o clock" -std=$C23 -DUART_BAUD=2000000 static_assert/preprocessador.c

echo "== erro sob medida (atributo error)"
if eh_clang; then
    echo "  pulado: o teste usa o comportamento do GCC com o atributo error"
else
    limpo  "-O2: a chamada inalcançável some" -std=$C23 -O2 erros/build_bug.c
    espera "-O0: a mesma chamada sobra e vira erro (falso positivo)" "ponto alcançável" -std=$C23 -O0 erros/build_bug.c
    espera "-O2 com chamada alcançável de verdade: erro" "ponto alcançável" -std=$C23 -O2 -DALCANCAVEL erros/build_bug.c
fi

echo "== análise estática (-fanalyzer)"
versao="$(echo __GNUC__ | "$CC" -E -P -x c - 2>/dev/null | tr -d ' ')"
if eh_clang; then
    echo "  pulado: -fanalyzer é do GCC (o Clang tem --analyze/scan-build)"
elif [ "${versao:-0}" -lt 14 ]; then
    echo "  pulado: o -fanalyzer do GCC $versao não pega este caso (pega a partir do GCC 14)"
else
    limpo  "o vazamento passa despercebido no -Wall -Wextra" -std=c17 -O2 $W analise/pool.c
    espera "o -fanalyzer acusa o vazamento do pool" "Wanalyzer-malloc-leak" -std=c17 -O2 -fanalyzer analise/pool.c
fi

echo "== Cortex-M4 (custo do double)"
if [ -z "${ARM_CC:-}" ]; then ARM_CC="$(command -v arm-none-eabi-gcc || true)"; fi
if [ -z "$ARM_CC" ]; then
    ARM_CC="$(ls ~/zephyr-sdk-*/gnu/arm-zephyr-eabi/bin/arm-zephyr-eabi-gcc \
                  ~/zephyr-sdk-*/arm-zephyr-eabi/bin/arm-zephyr-eabi-gcc 2>/dev/null | head -1 || true)"
fi
if [ -z "$ARM_CC" ]; then
    echo "  pulado: nenhum GCC para ARM (defina ARM_CC)"
else
    asm="$("$ARM_CC" -std=c17 -O2 -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -S avisos/double_promotion.c -o -)"
    d="$(awk '/^celsius:/{f=1;next} /^celsius_f:/{f=0} f' <<< "$asm" | grep -cE 'bl[[:space:]]+__aeabi_')"
    f="$(awk '/^celsius_f:/{f=1;next} /^[a-z_]+:/{f=0} f' <<< "$asm" | grep -cE 'bl[[:space:]]+__aeabi_')"
    if [ "$d" -ge 3 ] && [ "$f" -eq 0 ]; then
        ok "celsius() com 1.8 chama $d funções de double em software; celsius_f() com 1.8f, nenhuma"
    else
        falha "custo do double (celsius: $d chamadas, celsius_f: $f)"
    fi
    CC="$ARM_CC" espera "cortex-m0: o static_assert de lock-free quebra o build" "lock-free" -std=c2x -mcpu=cortex-m0 -mthumb static_assert/configuracao.c
fi

echo
if [ "$falhas" -eq 0 ]; then echo "tudo certo"; else echo "$falhas falha(s)"; exit 1; fi
