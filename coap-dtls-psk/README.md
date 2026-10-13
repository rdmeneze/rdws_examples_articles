# Exemplo: CoAP sobre DTLS com PSK

Acompanha o artigo [01-coap-dtls-psk](../../artigos/01-coap-dtls-psk/rascunho.md).

| Pasta | O que é | Estado |
|---|---|---|
| `servidor/` | Servidor CoAP/DTLS em C com libcoap 4.3.5 (OpenSSL): tabela fixa identidade → chave, POST em `/leituras` que loga a identidade da sessão e o corpo, timeout de sessão ociosa configurável | pronto |
| `scripts/` | `teste.sh`: identidade certa, chave errada e identidade desconhecida, com o `coap-client` do libcoap | pronto |
| `cliente-zephyr/` | App Zephyr com mbedTLS direto sobre socket UDP com `connect()`, handshake PSK, um POST CoAP com JSON em `/leituras` | pronto (testado em `native_sim`) |

## Servidor

Precisa de CMake 3.20+, um compilador C e o OpenSSL de desenvolvimento (`libssl-dev`). O
libcoap é baixado no configure, e o build gera também o `coap-client`.

```sh
cmake -S servidor -B servidor/build
cmake --build servidor/build -j
./servidor/build/servidor            # porta 5684, sessão ociosa expira em 60 s
./servidor/build/servidor 5684 300   # porta e timeout explícitos
```

## Teste

```sh
scripts/teste.sh
```

Saída esperada (resumida):

```
== identidade certa (espera 2.04)
POST de 'estacao-01': {"umidade":61.5,"unidade":"%RH"}
v:1 t:ACK c:2.04 ...
== chave errada (espera falha no handshake)
... ERR  cannot send CoAP pdu
== identidade desconhecida (espera recusa no log do servidor)
handshake recusado: identidade desconhecida 'estacao-99'
... DTLS: SSL3 alert write:fatal:unknown PSK identity
```

## Cliente Zephyr

Testado com Zephyr 4.4 e Zephyr SDK 1.0. No `native_sim` a rede usa os sockets do host
(NSOS, `boards/native_sim.conf`), sem TAP nem configuração no Linux.

```sh
west build -b native_sim -d cliente-zephyr/build cliente-zephyr
./servidor/build/servidor &
./cliente-zephyr/build/zephyr/zephyr.exe -stop_at=5
```

Saída esperada:

```
<inf> cliente: handshake OK como 'estacao-01', suíte TLS-PSK-WITH-AES-128-CCM-8
<inf> cliente: resposta 2.04
<inf> cliente: RESULTADO: OK
```

O aviso `Using a test - not safe - entropy source` é do gerador de entropia de teste do
`native_sim`. Numa placa, a entropia vem do RNG de hardware.

Servidor, porta e credenciais são opções de Kconfig (`Kconfig`). Para ver a chave errada
falhar:

```sh
west build -b native_sim -d build-errada cliente-zephyr -- -DCONFIG_CLIENTE_PSK_CHAVE=\"chave-errada\"
./build-errada/zephyr/zephyr.exe -stop_at=60
# <err> cliente: handshake falhou: -0x6800   (depois de ~38 s simulados)
```

O servidor não manda alerta para chave errada: o cliente só vê o handshake expirar.

## Critério de pronto da pasta toda

- `servidor/` aceita um POST com a identidade certa e recusa com a errada. ✅
- `cliente-zephyr/` fecha o handshake com `servidor/` e recebe `2.04`. ✅ (`native_sim`)
- Cada snippet do artigo existe, igual, num destes arquivos.
