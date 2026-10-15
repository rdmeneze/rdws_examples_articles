# Exemplos: extensões do GCC que facilitam a vida no firmware

Acompanha o artigo "Extensões do GCC que facilitam a vida no firmware" (link do Embarcados
quando for publicado). Cada arquivo é um programa pequeno, compilado com `-std=gnu17`.

| Pasta | O que tem |
|---|---|
| `atributos/` | `section` (`.noinit`), `packed` e `aligned`, `weak` + `alias` (handlers de interrupção, em `weak/`), `format(printf)`, `cleanup` (mutex liberado em todo caminho de saída) |
| `builtins/` | `__builtin_expect`, `bswap32`, `popcount`/`ctz`/`clz`, `add_overflow`, `constant_p`, `types_compatible_p` |
| `sintaxe/` | statement expressions com `__auto_type` (um `MIN` que avalia cada argumento uma vez) e intervalos em `case` e em inicializadores (`[0 ... 15] = default_handler`) |
| `asm/` | barreira de compilador (`barreira.c` + `espera_main.c`, que roda de verdade) e instruções do Cortex-M (`cpsid`, `wfi`) |
| `erros/` | o que as extensões pegam ou o que deve ser evitado: `format` errado, ponteiro para membro `packed`, `nonnull`, `warn_unused_result`, `-pedantic-errors`, função aninhada, array de tamanho zero, a macro `linux` do `-std=gnu17` |
| `compat/` | `compiler.h`: as extensões isoladas atrás de macros (`PACKED`, `WEAK`, `SECTION`, `PRINTF_FMT`, `likely`, `BSWAP32`), com `__has_attribute`/`__has_builtin` |
| `scripts/` | `teste.sh`: roda tudo e confere cada caso |

## Build e teste

```sh
make run          # compila e roda os exemplos
scripts/teste.sh  # também os casos de erros/, a barreira e (se houver GCC para ARM) o Cortex-M
CC=clang scripts/teste.sh
```

Testado com GCC 13 (host), GCC 15 (imagem `gcc:15`), Clang 22 (imagem `silkeh/clang:22`) e o
GCC 14 do Zephyr SDK 1.0.1 para Cortex-M0/M4. Os casos de função aninhada e da macro `linux`
são só do GCC e ficam de fora no Clang.

## A barreira de compilador

`asm/barreira.c` tem duas funções que esperam uma flag global (não `volatile`, de propósito)
mudar. Um sinal liga a flag depois de 200 ms, fazendo o papel de uma ISR:

```
GCC 13:    com barreira: retornou com flag = 1 / sem barreira: travou
Clang 22:  com barreira: retornou com flag = 1 / sem barreira: retornou com flag = 0
```

Sem a barreira, o compilador lê a flag uma vez só. O GCC transforma o laço em um laço
infinito; o Clang remove o laço (o C11, em 6.8.5p6, permite supor que um laço sem efeitos
colaterais termina) e a função retorna na hora. Os dois estão dentro do padrão, e os dois
quebram o firmware.
