#!/usr/bin/env bash
# Roda o app de zephyr/ nas placas QEMU do Zephyr, em dois modos:
#   padrao:        o QEMU só atende interrupções entre blocos de instruções traduzidos;
#   uma-instrucao: -accel tcg,one-insn-per-tb=on, um bloco por instrução, e a interrupção
#                  pode cair entre quaisquer duas instruções, como no hardware.
#
# Precisa do ambiente do Zephyr carregado (west, ZEPHYR_BASE e o Zephyr SDK, que traz o QEMU).
# Uso: scripts/qemu.sh [placas...]      (padrão: qemu_cortex_m0 qemu_cortex_m3)
set -uo pipefail

cd "$(dirname "$0")/.."
placas=("$@")
[ ${#placas[@]} -eq 0 ] && placas=(qemu_cortex_m0 qemu_cortex_m3)
LIMITE="${LIMITE:-900}"   # segundos por execução

scripts/baixa_neves.sh build/neves-src > /dev/null || exit 1

# roda <diretório de build> <arquivo de log>: inicia o QEMU e o encerra quando o app
# imprimir "=== FIM" (ou depois de LIMITE segundos).
roda() {
    local dir="$1" log="$2"
    setsid west build -d "$dir" -t run > "$log" 2>&1 &
    local pid=$!
    for _ in $(seq 1 "$LIMITE"); do
        grep -q '=== FIM' "$log" && break
        kill -0 "$pid" 2>/dev/null || break
        sleep 1
    done
    kill -- -"$pid" 2>/dev/null
    wait "$pid" 2>/dev/null
}

# O QEMU_EXTRA_FLAGS é lido pelo CMake na configuração do build, e não no "-t run": por isso
# há um diretório de build por modo, configurado com a variável já definida.
for placa in "${placas[@]}"; do
    for modo in padrao uma-instrucao; do
        dir="build/$placa-$modo"
        extra=""
        [ "$modo" = uma-instrucao ] && extra="-accel tcg,one-insn-per-tb=on"
        if ! QEMU_EXTRA_FLAGS="$extra" west build -p always -b "$placa" zephyr -d "$dir" \
                > "$dir.build.log" 2>&1; then
            echo "== $placa, modo $modo: falhou o build ($dir.build.log)"
            continue
        fi
        if [ -n "$extra" ] && ! grep -q 'one-insn-per-tb=on' "$dir/build.ninja"; then
            echo "== $placa, modo $modo: a opção não chegou ao comando do QEMU"
            continue
        fi
        log="$dir.log"
        inicio=$(date +%s)
        roda "$dir" "$log"
        echo "== $placa, modo $modo ($(( $(date +%s) - inicio )) s)"
        sed -n '/^contador\|^leitura\|^buffer/p' "$log" | sed 's/^/  /'
        grep -q '=== FIM' "$log" || echo "  (não terminou em ${LIMITE} s; veja $log)"
    done
done
