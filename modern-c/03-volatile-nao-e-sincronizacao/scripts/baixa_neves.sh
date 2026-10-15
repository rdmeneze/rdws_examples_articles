#!/usr/bin/env bash
# Baixa o buffer circular de Felipe Neves ("Buffer circular para sistemas embarcados",
# Embarcados, 2016) do repositório dele, sempre do mesmo commit, e confere o SHA-256.
# O código não é redistribuído aqui: o repositório original não declara licença.
#
# Uso: scripts/baixa_neves.sh <diretório de destino>
set -euo pipefail

destino="${1:?uso: $0 <diretório de destino>}"
commit=4c8f743af49094dcb394df990787eb2a512d55cd
url="https://raw.githubusercontent.com/uLipe/circular_buffer/$commit"

mkdir -p "$destino"
cd "$destino"
if sha256sum -c --quiet > /dev/null 2>&1 <<EOS
544a800590b2cf7d4d6310c546a4544abfb6c4b3719abc64387a8535e4f38994  buffer_circ.c
a1ea06703ba6f72a168b6bf222622ac5fa89255e18f8948e4e8046a00c57577b  buffer_circ.h
EOS
then
    exit 0   # já baixado e conferido
fi
for f in buffer_circ.c buffer_circ.h; do
    curl -sfL -m 60 -o "$f" "$url/$f"
done
sha256sum -c --quiet <<EOS
544a800590b2cf7d4d6310c546a4544abfb6c4b3719abc64387a8535e4f38994  buffer_circ.c
a1ea06703ba6f72a168b6bf222622ac5fa89255e18f8948e4e8046a00c57577b  buffer_circ.h
EOS
echo "baixado: uLipe/circular_buffer @ ${commit:0:7} em $destino"
