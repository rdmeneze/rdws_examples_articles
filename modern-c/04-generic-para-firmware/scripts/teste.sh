#!/usr/bin/env bash
# Testa os exemplos de _Generic no host e, se houver um GCC para ARM, as diferenças de tipo e o
# código gerado no Cortex-M.
#
# Uso: scripts/teste.sh            (CC=clang scripts/teste.sh)
#      ARM_CC=/caminho/arm-none-eabi-gcc scripts/teste.sh
set -uo pipefail

cd "$(dirname "$0")/.."
CC="${CC:-cc}"
falhas=0
ok()    { echo "  ok: $1"; }
falha() { echo "  FALHOU: $1"; falhas=$((falhas + 1)); }
if echo 'int x;' | "$CC" -std=c23 -x c -fsyntax-only - 2>/dev/null; then C23=c23; else C23=c2x; fi
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT

saida()  { "$1" "${@:2}" -c -o /dev/null 2>&1; }
espera() { local d="$1" p="$2"; shift 2; grep -q -- "$p" <<< "$(saida "$CC" "$@")" && ok "$d" || falha "$d"; }
limpo()  { local d="$1"; shift; local r; r="$(saida "$CC" "$@")"; [ -z "$r" ] && ok "$d" || { echo "$r" | head -3; falha "$d"; }; }
roda()   { "$CC" -std=$C23 -Wall -Wextra "$2" -o "$tmp/p" && "$tmp/p" > "$tmp/out" && grep -q -- "$3" "$tmp/out" && ok "$1" || { cat "$tmp/out" 2>/dev/null; falha "$1"; }; }

echo "== pegadinhas (host)"
"$CC" -std=c11 -Wall pegadinhas/nome_do_tipo.c -o "$tmp/nome" && "$tmp/nome" | sed 's/^/  /'
r="$("$tmp/nome")"
grep -q "'a'      -> int" <<< "$r" && ok "'a' é int" || falha "'a'"
grep -q "ch + 1   -> int" <<< "$r" && ok "char + 1 é int (promoção)" || falha "char + 1"
grep -q "const int-> int" <<< "$r" && ok "o const é descartado" || falha "const"
grep -q "uint8_t  -> unsigned char" <<< "$r" && ok "uint8_t é unsigned char (typedef, não tipo novo)" || falha "uint8_t"

echo "== aplicações (host)"
limpo  "REG_WRITE compila sem avisos" -std=$C23 -Wall -Wextra aplicacoes/registradores.c
espera "REG_WRITE com ponteiro de outro tipo: erro" "not compatible with any association\|controlling expression type" -std=$C23 -DTIPO_ERRADO aplicacoes/registradores.c
roda   "LOG_VAL imprime cada tipo com o formato certo" aplicacoes/log.c "a = 4000000000"
limpo  "LOG_VAL sem avisos com -Wformat=2" -std=$C23 -Wall -Wextra -Wformat=2 aplicacoes/log.c
sed 's/unsigned long: "%lu"/unsigned long: "%d"/' aplicacoes/log.c > "$tmp/log_errado.c"
espera "o -Wformat confere o formato escolhido pelo _Generic" "Wformat" -std=$C23 -Wall -Wformat=2 "$tmp/log_errado.c"
roda   "PUT_BE grava em big-endian" aplicacoes/serializacao.c "01 02 03 04 05 06"
espera "PUT_BE com um literal int: erro" "not compatible with any association\|controlling expression type" -std=$C23 -DLITERAL aplicacoes/serializacao.c
limpo  "EXIGE_TIPO com o tipo certo compila" -std=$C23 aplicacoes/exige_tipo.c
espera "EXIGE_TIPO com o tipo errado: erro com a mensagem" "contador precisa ser uint32_t" -std=$C23 -DERRADO aplicacoes/exige_tipo.c

echo "== int32_t: host x ARM"
r="$(saida "$CC" -std=c11 pegadinhas/int32_t.c)"
if grep -qi "compatible\|duplicate\|two compatible" <<< "$r"; then
    ok "host: int e int32_t juntos é erro (int32_t é int aqui)"
else
    echo "$r" | head -2; falha "host: esperava erro de tipos compatíveis"
fi

if [ -z "${ARM_CC:-}" ]; then ARM_CC="$(command -v arm-none-eabi-gcc || true)"; fi
if [ -z "$ARM_CC" ]; then
    ARM_CC="$(ls ~/zephyr-sdk-*/gnu/arm-zephyr-eabi/bin/arm-zephyr-eabi-gcc \
                  ~/zephyr-sdk-*/arm-zephyr-eabi/bin/arm-zephyr-eabi-gcc 2>/dev/null | head -1 || true)"
fi
if [ -z "$ARM_CC" ]; then
    echo "  pulado: nenhum GCC para ARM (defina ARM_CC)"
else
    echo "  usando $("$ARM_CC" --version | head -1)"
    if "$ARM_CC" -std=c11 -O2 -mcpu=cortex-m4 -mthumb -S pegadinhas/int32_t.c -o "$tmp/i.s" 2>/dev/null; then
        ok "ARM: int e int32_t juntos compila (int32_t é long)"
        grep -q '"default' "$tmp/i.s" && ok "ARM: uint32_t cai no default de um _Generic que só conhece int" || falha "ARM: uint32_t no default"
    else
        falha "ARM: esperava compilar"
    fi
    asm="$("$ARM_CC" -std=c11 -O2 -mcpu=cortex-m0 -mthumb -S aplicacoes/registradores.c -o - | grep -oE '^\s+str[bh]?\s' | tr -d ' \t' | tr '\n' ' ')"
    [ "$asm" = "str strh strb " ] && ok "cortex-m0: REG_WRITE gera str, strh e strb" || falha "cortex-m0: REG_WRITE ($asm)"
    "$ARM_CC" -std=c11 -Wall -Wformat=2 -c aplicacoes/log.c -o /dev/null 2>"$tmp/w" && [ ! -s "$tmp/w" ] \
        && ok "ARM: LOG_VAL compila sem avisos (uint32_t recebe %lu sozinho)" || { cat "$tmp/w"; falha "ARM: LOG_VAL"; }
    asm="$("$ARM_CC" -std=c11 -O2 -mcpu=cortex-m4 -mthumb -S aplicacoes/serializacao.c -o - | grep -oE '^\s+rev(16)?\s' | tr -d ' \t' | sort -u | tr '\n' ' ')"
    [ "$asm" = "rev rev16 " ] && ok "cortex-m4: PUT_BE usa rev e rev16" || falha "cortex-m4: PUT_BE ($asm)"
    FPU="-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16"
    m="$("$ARM_CC" -std=c11 -O2 $FPU -S biblioteca/math_h.c -o - | grep -cE 'bl\s+(sqrt|__aeabi_)')"
    t="$("$ARM_CC" -std=c11 -O2 $FPU -S biblioteca/tgmath_h.c -o -)"
    if [ "$m" -ge 3 ] && grep -q 'vsqrt.f32' <<< "$t" && ! grep -qE 'bl\s' <<< "$t"; then
        ok "cortex-m4: sqrt de float com math.h = $m chamadas em double; com tgmath.h = um vsqrt.f32"
    else
        falha "math.h x tgmath.h"
    fi
fi

echo
if [ "$falhas" -eq 0 ]; then echo "tudo certo"; else echo "$falhas falha(s)"; exit 1; fi
