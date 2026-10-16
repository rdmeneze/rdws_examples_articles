# Exemplos: usando _Generic para criar APIs genéricas em C

Acompanha o artigo "Usando _Generic para criar APIs genéricas em C" (link do Embarcados quando
for publicado).

| Pasta | O que tem |
|---|---|
| `pegadinhas/` | Qual tipo o `_Generic` enxerga (`nome_do_tipo.c`: literais, promoções, `const`, typedefs) e o `int32_t`, que é `int` no x86 e `long` no ARM (`int32_t.c`) |
| `aplicacoes/` | Escrita em registrador pela largura do ponteiro (`registradores.c`), log com o formato do `printf` escolhido pelo tipo (`log.c`), serialização big-endian (`serializacao.c`) e verificação de tipo com `static_assert` (`exige_tipo.c`) |
| `biblioteca/` | `sqrt` de um `float` com `math.h` e com `tgmath.h`, para comparar o código gerado no Cortex-M4 |
| `scripts/teste.sh` | Roda e confere tudo no host e, com um GCC para ARM, as diferenças de tipo e o código gerado |

## Rodando

```sh
scripts/teste.sh
CC=clang scripts/teste.sh
```

Testado com GCC 13 (host), GCC 15 (imagem `gcc:15`), Clang 22 (imagem `silkeh/clang:22`) e o
GCC 14 do Zephyr SDK 1.0.1 para Cortex-M0 e M4.

## O que vale saber

- O `_Generic` não aplica as promoções inteiras: um `short` não casa com `int`. O truque
  `_Generic((x) + 0, ...)` aplica as promoções antes da escolha.
- Os typedefs de tamanho fixo não são tipos novos: `uint8_t` é `unsigned char`, e o
  `int32_t` é `int` no x86 com glibc, mas `long` no `arm-none-eabi` e no Zephyr SDK. Um
  `_Generic` que lista `int` e `int32_t` juntos não compila no PC e compila no ARM.
- Sem `default`, um tipo não previsto vira erro de compilação, e não um comportamento silencioso.
