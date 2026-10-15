#!/usr/bin/env bash
# Testa cada recurso de testes/ em um compilador e imprime uma linha por recurso, separada
# por tabulações: toolchain, recurso, resultado.
#
# Uso: scripts/matriz.sh <nome> <compilador> [flags extras...]
#   ALVO=host (padrão): os testes do tipo "roda" são executados.
#   ALVO=cross: os testes do tipo "roda" ficam como "n/t" (não testado).
#   LINK="..."   flags extras só para o link (ex.: --specs=nosys.specs).
#
# Resultados: ok | não (não compila) | só declara (compila, mas não linka) | só gnu (recurso
# de biblioteca que a libc só expõe com -std=gnu23/gnu2x) | falha (roda com saída errada) |
# n/t (teste de execução em alvo cruzado).
set -uo pipefail

nome="$1"; cc="$2"; shift 2
extra=("$@")
alvo="${ALVO:-host}"
link=(${LINK:-})
cd "$(dirname "$0")/../testes"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT

# Usa -std=c23 se o compilador aceitar; senão, o nome provisório c2x.
if echo 'int x;' | "$cc" -std=c23 "${extra[@]}" -x c -fsyntax-only - 2>/dev/null; then
    std=c23
else
    std=c2x
fi
versao="$(echo | "$cc" -std=$std "${extra[@]}" -x c -dM -E - | awk '/__STDC_VERSION__/ {print $3}')"
macros="$(echo '#include <stdio.h>' | "$cc" -std=$std "${extra[@]}" -x c -dM -E - 2>/dev/null)"
m() { awk -v n="$1" '$1 == "#define" && $2 == n { $1 = ""; $2 = ""; sub(/^  /, ""); gsub(/"/, ""); print }' <<< "$macros"; }
if [ -n "$(m __clang_major__)" ]; then
    compilador="Clang $(m __clang_major__).$(m __clang_minor__).$(m __clang_patchlevel__)"
else
    compilador="GCC $(m __GNUC__).$(m __GNUC_MINOR__).$(m __GNUC_PATCHLEVEL__)"
fi
if [ -n "$(m __GLIBC__)" ]; then
    libc="glibc $(m __GLIBC__).$(m __GLIBC_MINOR__)"
elif [ -n "$(m __PICOLIBC_VERSION__)" ]; then
    libc="picolibc $(m __PICOLIBC_VERSION__)"
elif [ -n "$(m _NEWLIB_VERSION)" ]; then
    libc="newlib $(m _NEWLIB_VERSION)"
else
    libc="?"
fi
printf '%s\t%s\t%s\n' "$nome" "_compilador" "$compilador"
printf '%s\t%s\t%s\n' "$nome" "_libc" "$libc"
printf '%s\t%s\t%s\n' "$nome" "_std" "-std=$std ($versao)"

flags=(-std=$std -pedantic-errors -Werror "${extra[@]}")

# testa <flags...>: compila e, se o teste pedir, linka e roda. Imprime o resultado.
testa() {
    if ! "$cc" "$@" -c "$f" -o "$tmp/t.o" > "$tmp/log" 2>&1; then
        echo "não"
    elif [ "$verifica" = compila ]; then
        echo "ok"
    elif [ "$verifica" = roda ] && [ "$alvo" != host ]; then
        echo "n/t"
    elif ! "$cc" "${extra[@]}" "$tmp/t.o" -o "$tmp/t.elf" "${link[@]}" > "$tmp/log" 2>&1; then
        echo "só declara"
    elif [ "$verifica" = linka ]; then
        echo "ok"
    elif [ "$("$tmp/t.elf")" = "$esperado" ]; then
        echo "ok"
    else
        echo "falha"
    fi
}

for f in *.c; do
    cab="$(head -1 "$f")"
    tipo="$(sed -n 's/.*tipo: \([^;]*\);.*/\1/p' <<< "$cab")"
    verifica="$(sed -n 's/.*verifica: \([a-z]*\).*/\1/p' <<< "$cab")"
    esperado="$(sed -n 's/.*esperado: \([^;]*\);.*/\1/p' <<< "$cab")"

    res="$(testa "${flags[@]}")"
    # Na libc, "não" pode ser só a visibilidade: a função existe, mas o cabeçalho a esconde
    # no modo estrito. Tenta de novo com as extensões GNU.
    if [ "$res" = "não" ] && [ "$tipo" = biblioteca ]; then
        if [ "$(testa -std=gnu${std#c} "${extra[@]}")" = ok ]; then
            res="só gnu"
        fi
    fi
    printf '%s\t%s\t%s\n' "$nome" "${f%.c}" "$res"
done
