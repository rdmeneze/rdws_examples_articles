#!/usr/bin/env bash
# Testa os exemplos: compila e roda c99/, c11/ e c23/, confere que os casos de erros/
# falham com a mensagem esperada e, se houver um GCC para ARM, compara os atomics no
# Cortex-M0 e no Cortex-M4.
#
# Uso: scripts/teste.sh            (CC=clang scripts/teste.sh para outro compilador)
#      ARM_CC=/caminho/arm-none-eabi-gcc scripts/teste.sh
set -uo pipefail

cd "$(dirname "$0")/.."
CC="${CC:-cc}"
STD_C23="${STD_C23:-c2x}"
falhas=0

ok()    { echo "  ok: $1"; }
falha() { echo "  FALHOU: $1"; falhas=$((falhas + 1)); }

echo "== exemplos (make run)"
log="$(mktemp)"
if make -s clean && make -s CC="$CC" STD_C23="$STD_C23" run > "$log" 2>&1; then
    ok "todos compilam e rodam"
else
    cat "$log"
    falha "make run"
fi
rm -f "$log"

# espera_falha <descrição> <padrão esperado na saída> <argumentos do compilador...>
espera_falha() {
    local desc="$1" padrao="$2"; shift 2
    local saida
    if saida="$("$CC" "$@" -o /dev/null 2>&1)"; then
        falha "$desc (compilou, mas devia falhar)"
    elif grep -q -- "$padrao" <<< "$saida"; then
        ok "$desc"
    else
        echo "$saida" | head -5
        falha "$desc (falhou, mas sem \"$padrao\")"
    fi
}

# espera_sucesso <descrição> <argumentos do compilador...>
espera_sucesso() {
    local desc="$1"; shift
    local saida
    if saida="$("$CC" "$@" -o /dev/null 2>&1)"; then
        ok "$desc"
    else
        echo "$saida" | head -5
        falha "$desc (devia compilar)"
    fi
}

W="-Wall -Wextra -pedantic"

echo "== casos que devem falhar (erros/)"
espera_falha   "LOG sem argumentos no C99" "expected expression" \
               -std=c99 $W erros/log_sem_argumentos.c
espera_falha   "VLA com -Werror=vla" "variable length array" \
               -std=c99 $W -Werror=vla erros/vla.c
espera_falha   "tabela de sensores incompleta" "sensor_cfg precisa ter uma entrada por sensor" \
               -std="$STD_C23" $W erros/tabela_sensores.c
espera_sucesso "tabela de sensores completa" \
               -std="$STD_C23" $W -DTABELA_COMPLETA erros/tabela_sensores.c
espera_falha   "[[nodiscard]] ignorado com -Werror=unused-result" "ignoring return value" \
               -std="$STD_C23" $W -Werror=unused-result erros/nodiscard_ignorado.c
espera_sucesso "void f(); chamado com argumento no C17" \
               -std=c17 erros/f_sem_parametros.c
espera_falha   "void f(); chamado com argumento no C23" "too many arguments" \
               -std="$STD_C23" erros/f_sem_parametros.c

echo "== atomics no Cortex-M0 e no Cortex-M4"
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
    CC="$ARM_CC" espera_sucesso "Cortex-M4: ATOMIC_INT_LOCK_FREE == 2" \
        -std=c11 -mcpu=cortex-m4 -mthumb -c c11/atomics.c
    CC="$ARM_CC" espera_falha   "Cortex-M0: _Static_assert falha" "precisa ser lock-free" \
        -std=c11 -mcpu=cortex-m0 -mthumb -c c11/atomics.c

    for cpu in cortex-m4 cortex-m0; do
        asm="$("$ARM_CC" -std=c11 -mcpu=$cpu -mthumb -O2 -DSEM_ASSERT_LOCK_FREE -S c11/atomics.c -o - \
               | sed -n '/^uart_isr:/,/\.size[[:space:]]*uart_isr/p')"
        case "$cpu" in
        cortex-m4) esperado="ldrex" ;;
        cortex-m0) esperado="__atomic_fetch_add_4" ;;
        esac
        if grep -q "$esperado" <<< "$asm"; then
            ok "$cpu: uart_isr usa $esperado"
        else
            echo "$asm"
            falha "$cpu: uart_isr sem $esperado"
        fi
    done
fi

make -s clean
echo
if [ "$falhas" -eq 0 ]; then
    echo "tudo certo"
else
    echo "$falhas falha(s)"
    exit 1
fi
