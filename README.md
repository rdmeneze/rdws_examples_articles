# rdws_examples_articles

Exemplos de código dos artigos técnicos de Rafael Dias: a série sobre o projeto rdws, uma
estação meteorológica com firmware Zephyr e backend próprio, e a série Modern C, sobre a
evolução da linguagem C e o que ela muda para firmware. Cada pasta acompanha um artigo e traz
um exemplo mínimo e completo, que compila e roda sozinho.

| Pasta | Artigo | O que tem |
|---|---|---|
| [coap-dtls-psk](coap-dtls-psk/) | CoAP sobre DTLS com PSK no Zephyr: do sensor ao servidor | Servidor CoAP/DTLS em C (libcoap), cliente Zephyr com mbedTLS direto (`native_sim`) e script de teste |
| [modern-c/01-c99-a-c23](modern-c/01-c99-a-c23/) | 25 anos de evolução da linguagem C: de C99 a C23 | Um programa por recurso de C99, C11 e C23, com Makefile; casos que devem falhar e teste automático |

Instruções de build e teste ficam no README de cada pasta.

## Licença

[MIT](LICENSE).
