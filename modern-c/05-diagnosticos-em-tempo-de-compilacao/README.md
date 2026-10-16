# Exemplos: diagnósticos em tempo de compilação

Acompanha o artigo "Diagnósticos em tempo de compilação em C" (link do Embarcados quando for
publicado). Cada arquivo mostra um diagnóstico; o teste confere que ele aparece com a opção certa
e não aparece sem ela.

| Pasta | O que tem |
|---|---|
| `static_assert/` | Premissas de configuração e de plataforma (`configuracao.c`: tamanho potência de 2, `CHAR_BIT`, atomics lock-free, pacote que cabe no payload) e o que o pré-processador enxerga (`preprocessador.c`: `#error` e o `#warning` do C23) |
| `atributos/` | `[[nodiscard]]` e `[[deprecated]]` com mensagem, e o `fallthrough` sem nada, com comentário e com o atributo |
| `avisos/` | O que o `-Wall -Wextra` não liga: `-Wconversion`, `-Wdouble-promotion`, `-Wundef`, `-Wswitch-enum`, `-Wshadow` |
| `erros/` | Erro sob medida com o atributo `error` do GCC, e o falso positivo em `-O0` |
| `analise/` | Um vazamento num alocador de pool que só o `-fanalyzer` do GCC 14+ acusa, graças ao atributo `malloc(pool_free, 1)` |
| `scripts/teste.sh` | Confere tudo e, com um GCC para ARM, o custo do `double` no Cortex-M4 |

## Rodando

```sh
scripts/teste.sh
CC=clang scripts/teste.sh
```

Testado com GCC 13 (host), GCC 15 (imagem `gcc:15`), Clang 22 (imagem `silkeh/clang:22`) e o
GCC 14 do Zephyr SDK 1.0.1 para Cortex-M0 e M4.

## Diferenças entre compiladores que o teste leva em conta

- No Clang, o `-Wimplicit-fallthrough` não faz parte do `-Wextra`, e o comentário
  `/* fall through */` não cala o aviso: só o atributo.
- No Clang, o truncamento aparece como `-Wimplicit-int-conversion`, que faz parte do
  `-Wconversion`.
- O `-fanalyzer` do GCC 13 não pega o vazamento de `analise/pool.c`; do GCC 14 em diante, pega.
- O GCC (até o 16, pelo menos) mostra os acentos da mensagem de um `[[nodiscard]]` como escapes
  octais; o Clang mostra certo. Mensagens sem acento evitam o problema.
