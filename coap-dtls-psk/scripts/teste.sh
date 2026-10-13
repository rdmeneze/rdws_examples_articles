#!/usr/bin/env bash
# Testa o servidor com o coap-client do libcoap: uma identidade certa e uma errada.
# Uso: scripts/teste.sh [diretorio_do_build]   (padrão: servidor/build)
set -euo pipefail

aqui="$(cd "$(dirname "$0")/.." && pwd)"
build="${1:-$aqui/servidor/build}"
cliente="$build/_deps/libcoap-build/coap-client"
porta=5699

"$build/servidor" "$porta" &
servidor_pid=$!
trap 'kill "$servidor_pid" 2>/dev/null || true' EXIT
sleep 1

# Payload num arquivo: com -e, o coap-client decodifica '%' como percent-encoding e um
# corpo com "%RH" vira corpo vazio, sem aviso.
payload="$(mktemp)"
printf '{"umidade":61.5,"unidade":"%%RH"}' > "$payload"

echo "== identidade certa (espera 2.04)"
"$cliente" -m post -u estacao-01 -k segredo-da-estacao-01 -f "$payload" \
    "coaps://127.0.0.1:$porta/leituras" -v 6 2>&1 | grep -E "t:ACK" || true

echo "== chave errada (espera falha no handshake)"
"$cliente" -m post -u estacao-01 -k chave-errada -f "$payload" -B 5 \
    "coaps://127.0.0.1:$porta/leituras" 2>&1 | tail -2 || true

echo "== identidade desconhecida (espera recusa no log do servidor)"
"$cliente" -m post -u estacao-99 -k qualquer -f "$payload" -B 5 \
    "coaps://127.0.0.1:$porta/leituras" 2>&1 | tail -2 || true

rm -f "$payload"
