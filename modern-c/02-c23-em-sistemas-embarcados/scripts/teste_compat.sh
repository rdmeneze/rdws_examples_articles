#!/usr/bin/env bash
# Compila e roda compat/exemplo.c com cada padrão de C11 a C23 aceito pelo compilador.
# Uso: scripts/teste_compat.sh            (CC=clang scripts/teste_compat.sh)
set -uo pipefail
cd "$(dirname "$0")/../compat"
CC="${CC:-cc}"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
falhas=0
for std in c11 c17 c2x c23; do
    if ! echo 'int x;' | "$CC" -std=$std -x c -fsyntax-only - 2>/dev/null; then
        echo "-std=$std: não aceito por este compilador"
        continue
    fi
    if "$CC" -std=$std -Wall -Wextra -Werror exemplo.c -o "$tmp/ex" 2> "$tmp/log"; then
        printf -- '-std=%s: %s\n' "$std" "$("$tmp/ex" | tail -1)"
    else
        cat "$tmp/log"; echo "-std=$std: FALHOU"; falhas=$((falhas + 1))
    fi
done
# A abertura do artigo: o builtin sempre; o ckd_add só onde houver stdckdint.h.
if "$CC" -std=c2x -Wall -Wextra -Werror -c overflow.c -o "$tmp/o.o" 2> "$tmp/log"; then
    echo "overflow.c: $(nm "$tmp/o.o" | grep -o 'soma_[a-z0-9]*' | sort | tr '\n' ' ')"
else
    cat "$tmp/log"; echo "overflow.c: FALHOU"; falhas=$((falhas + 1))
fi
exit $falhas
