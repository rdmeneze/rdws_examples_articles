# rdws_examples_articles

Exemplos de código dos artigos técnicos sobre o projeto rdws, uma estação meteorológica com
firmware Zephyr e backend próprio. Cada pasta acompanha um artigo e traz um exemplo mínimo e
completo, que compila e roda sozinho.

| Pasta | Artigo | O que tem |
|---|---|---|
| [coap-dtls-psk](coap-dtls-psk/) | CoAP sobre DTLS com PSK no Zephyr: do sensor ao servidor | Servidor CoAP/DTLS em C (libcoap), cliente Zephyr com mbedTLS direto (`native_sim`) e script de teste |

Instruções de build e teste ficam no README de cada pasta.

## Licença

[MIT](LICENSE).
