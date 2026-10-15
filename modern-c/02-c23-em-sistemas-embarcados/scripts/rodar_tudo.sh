#!/usr/bin/env bash
# Roda scripts/matriz.sh em todos os toolchains e monta resultados/matriz.xlsx (precisa do
# openpyxl: pip install openpyxl, ou o pacote python3-openpyxl da distribuição).
#
# Host: imagens Docker (gcc:13 a gcc:16, silkeh/clang:18 a 22 e fedora:44, que traz a
# glibc 2.43; nela o GCC é instalado com dnf na hora).
# ARM: as Arm GNU Toolchains listadas em ARM_GNU (diretórios de instalação, separados por
# espaço) e o Zephyr SDK em ZEPHYR_SDK. Toolchains não encontrados são pulados.
set -euo pipefail

cd "$(dirname "$0")/.."
mkdir -p resultados
rm -f resultados/*.tsv

n=0
roda() {   # roda <arquivo> <comando...>
    n=$((n + 1))
    local saida
    saida="resultados/$(printf '%02d' $n)-$1.tsv"; shift
    echo "== $saida"
    "$@" > "$saida"
}

for img in gcc:13 gcc:14 gcc:15 gcc:16; do
    roda "${img/:/-}" docker run --rm -v "$PWD":/src:ro -w /src "$img" \
        scripts/matriz.sh "$img" gcc
done
for v in 18 19 20 21 22; do
    roda "clang-$v" docker run --rm -v "$PWD":/src:ro -w /src "silkeh/clang:$v" \
        scripts/matriz.sh "clang:$v" clang
done

roda "fedora-44" docker run --rm -v "$PWD":/src:ro -w /src fedora:44 \
    sh -c 'dnf -y -q install gcc > /dev/null 2>&1 && scripts/matriz.sh "fedora:44" gcc'

ARM="-mcpu=cortex-m4 -mthumb"
for dir in ${ARM_GNU:-}; do
    cc="$dir/bin/arm-none-eabi-gcc"
    [ -x "$cc" ] || { echo "pulado: $cc"; continue; }
    roda "arm-gnu-$(basename "$dir")" env ALVO=cross LINK="--specs=nosys.specs" \
        scripts/matriz.sh "arm-none-eabi $(basename "$dir")" "$cc" $ARM
done
if [ -n "${ZEPHYR_SDK:-}" ]; then
    cc="$(ls "$ZEPHYR_SDK"/gnu/arm-zephyr-eabi/bin/arm-zephyr-eabi-gcc \
              "$ZEPHYR_SDK"/arm-zephyr-eabi/bin/arm-zephyr-eabi-gcc 2>/dev/null | head -1 || true)"
    if [ -n "$cc" ]; then
        roda "zephyr-sdk" env ALVO=cross \
            scripts/matriz.sh "Zephyr SDK $(basename "$ZEPHYR_SDK" | sed 's/zephyr-sdk-//')" "$cc" $ARM
    else
        echo "pulado: Zephyr SDK em $ZEPHYR_SDK"
    fi
fi

python3 scripts/tabela.py resultados/matriz.xlsx resultados/*.tsv
