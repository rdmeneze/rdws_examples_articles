# Exemplos: 25 anos de evolução da linguagem C, de C99 a C23

Acompanha o artigo "25 anos de evolução da linguagem C: de C99 a C23" (link do Embarcados
quando for publicado). Cada arquivo é um programa pequeno, que compila sozinho e imprime o
que o recurso faz. Os trechos citados no artigo são idênticos aos destes arquivos.

| Pasta | Padrão | O que tem | Estado |
|---|---|---|---|
| `c99/` | `-std=c99` | inicializadores designados (structs e vetores), literais compostos, flexible array member, macros variádicas, `restrict`, `stdint.h`/`inttypes.h` | pronto |
| `c11/` | `-std=c11` | `_Static_assert` de layout, `alignas`, structs e unions anônimas, atomics, `_Generic`, `_Noreturn` | pronto |
| `c23/` | `-std=c2x` (ou `c23`) | `bool` e `nullptr`, atributos, literais binários e enums com tipo fixo, `constexpr`/`typeof`/`auto`/`= {}`, `__VA_OPT__` e `#embed`, `ckd_add` e `memset_explicit`, `stdbit.h` e `unreachable` | pronto |
| `erros/` | vários | casos que devem falhar: `LOG` sem argumentos no C99, VLA com `-Wvla`, tabela de sensores incompleta, `[[nodiscard]]` ignorado | a fazer |
| `scripts/` | | `teste.sh`: compila tudo, roda e confere que os casos de erro falham | a fazer |

## Build

Precisa de `make` e de um GCC ou Clang recente. Alguns recursos do C23 dependem da versão do
compilador ou da libc; quando faltam, o exemplo compila assim mesmo e avisa "não suportado":

| Recurso | Precisa de |
|---|---|
| `#embed` | GCC 15 ou Clang 19 |
| `ckd_add` (`stdckdint.h`) | GCC 14 ou Clang 18 |
| `memset_explicit` | uma libc que já tenha a função (a glibc 2.41 ainda não tem); o Makefile testa e define `HAVE_MEMSET_EXPLICIT` |

Testado com GCC 13.3 (Ubuntu 24.04, glibc 2.39) e GCC 15.3 (imagem Docker `gcc:15`, glibc 2.41),
sem avisos com `-Wall -Wextra -pedantic`.

```sh
make          # compila c99/, c11/ e c23/
make run      # compila e roda todos os exemplos
make clean
```

Para usar outro compilador: `make CC=clang`. Com GCC 14 ou mais novo dá para usar o nome
oficial do padrão: `make STD_C23=c23`. Para testar com o GCC 15 sem instalá-lo:

```sh
docker run --rm -v "$PWD":/src:ro -w /src gcc:15 make BUILD=/tmp/build STD_C23=c23 run
```

## Saída esperada (resumida, GCC 15 em x86-64)

```
== build/c99/flexible_array
sizeof(packet_t) = 2 (só o cabeçalho)
pacote: id=7 len=5 payload=21.5C
== build/c99/inicializadores
cfg: baudrate=115200 parity=0 stop_bits=1 flow_control=0
led_pin: [LED_STATUS]=13 [LED_ERROR]=12
send_message: id=1 len=8
...
== build/c11/atomics
rx_count = 5
ATOMIC_INT_LOCK_FREE = 2 (0 nunca, 1 às vezes, 2 sempre)
atomic_is_lock_free(&rx_count) = 1
...
== build/c11/static_assert_layout
sizeof(packet_header_t) = 8
offsetof(crc) = 2, offsetof(timestamp) = 4
...
== build/c23/bool_nullptr
device_init(nullptr)
enabled = 1
NULL nesta implementação é do tipo: void *
nullptr é do tipo: nullptr_t
...
== build/c23/preprocessador
iniciando
valor = 42
ca_cert: 396 bytes, começa com 0x30 0x82 (sequência DER)
== build/c23/seguranca
total_len(8, 100)   -> 0
total_len(8, 65530) -> -75 (-EOVERFLOW = -75)
memset_explicit: não disponível nesta libc
...
```

O `c23/certs/ca.der` é um certificado autoassinado gerado só para o exemplo do `#embed`; a
chave privada foi descartada.

No x86-64, `ATOMIC_INT_LOCK_FREE` vale 2. Em um Cortex-M0 vale 1, e o `_Static_assert` de
`c11/atomics.c` falha na compilação; o alvo `arm`, na etapa 3, mostra isso.
