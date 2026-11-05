# Medir o custo do ECDH P-256 num Cortex-M4

Mede o que o ECDHE-PSK acrescenta ao PSK puro no lado do cliente: gerar um par de chaves P-256
efêmero, exportar a chave pública e fazer o acordo de chaves. Faz 10 rodadas e imprime tempo, pico
do heap do mbedTLS e pico de pilha. Não mede o handshake inteiro.

`gerar` inclui a exportação de propósito: na implementação genérica do mbedTLS, a chave pública só
é calculada nesse momento. Sem ela, a conta perde uma multiplicação de ponto.

Alvo: NUCLEO-L432KC (Cortex-M4 a 80 MHz). Testado com Zephyr 4.4 e Zephyr SDK 1.0.

## Compilar e gravar

```sh
west build -p always -b nucleo_l432kc -d build-mbedtls .
west flash -d build-mbedtls --runner openocd

west build -p always -b nucleo_l432kc -d build-p256m . -- -DEXTRA_CONF_FILE=p256m.conf
west flash -d build-p256m --runner openocd
```

Se o ST-Link da placa tiver firmware J-Link, use `--runner jlink`.

`p256m.conf` troca a implementação genérica de curvas do mbedTLS pelo driver p256-m.

## Ler o resultado

A saída vai pela porta serial do ST-Link, a 115200 baud:

```sh
picocom -b 115200 /dev/ttyACM0
```

Aperte o reset da placa para rodar de novo. As linhas que importam começam com `RESULTADO:`.

## No `native_sim`

```sh
west build -p always -b native_sim -d build-sim .
./build-sim/zephyr/zephyr.exe -stop_at=30
```

Serve para conferir o programa e o pico de heap. O tempo sai zerado, porque é simulado, e a
pilha não vale, porque a thread roda sobre a pilha do host.
