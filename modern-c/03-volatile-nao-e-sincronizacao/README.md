# Exemplos: volatile não é sincronização

Acompanha o artigo "volatile não é sincronização: usando atomics do C11 em firmware" (link do
Embarcados quando for publicado).

| Pasta | O que tem |
|---|---|
| `host/` | Programas que rodam no Linux, com um sinal `SIGALRM` a cada 20 µs fazendo o papel da ISR (`isr.h`): incrementos perdidos com `volatile` e sem perda com `atomic` (`rmw.c`), leitura rasgada de um contador de 64 bits e duas correções (`rasgada.c`), ring buffer de um produtor e um consumidor com acquire/release (`ringbuf.h`, `ring.c`); o buffer circular de Felipe Neves (Embarcados, 2016) entre a "ISR" e o código principal (`neves.c`) |
| `arm/` | Arquivos que só compilam para Cortex-M; o teste confere o assembly: `flags \|= BIT` (`rmw.c`), leitura de 64 bits (`ticks64.c`), reordenação em volta de um `volatile` (`reordena.c`), DMBs por ordem de memória (`ordens.c`), `atomic_fetch_add` no M0 (`fetch_add.c`), o ring buffer no M0 (`ring.c`) e seção crítica com PRIMASK (`secao_critica.c`) |
| `zephyr/` | App Zephyr com as mesmas experiências (contador `volatile` e `atomic_t`, leitura de 64 bits, buffer do Neves), com a interrupção de um `k_timer` a 10 kHz, para rodar nas placas `qemu_cortex_m0` e `qemu_cortex_m3` |
| `scripts/qemu.sh` | Compila e roda o app no QEMU em dois modos: padrão e `-accel tcg,one-insn-per-tb=on` |
| `scripts/baixa_neves.sh` | Baixa o `buffer_circ.c`/`.h` de [uLipe/circular_buffer](https://github.com/uLipe/circular_buffer), sempre do commit `4c8f743`, e confere o SHA-256. O código não é redistribuído aqui, porque o repositório original não declara licença; o `make` e o teste chamam o script sozinhos (precisa de rede na primeira vez) |
| `scripts/teste.sh` | Roda os programas de host e confere o ARM, se houver um GCC para ARM |

## Rodando

```sh
make run            # programas de host
scripts/teste.sh    # tudo, inclusive o assembly do Cortex-M
CC=clang scripts/teste.sh
```

Testado com GCC 13 (host), GCC 15 (imagem `gcc:15`), Clang 22 (imagem `silkeh/clang:22`) e o
GCC 14 do Zephyr SDK 1.0.1 para Cortex-M0 e M4.

## Saída típica (GCC 13 no host, GCC 14 do Zephyr SDK)

```
volatile: esperado 300009032, obtido 300005893, incrementos perdidos: 3139
atomic: esperado 300062980, obtido 300062980, incrementos perdidos: 0
ingenua: o contador pareceu andar para trás 563 vezes
repete: o contador pareceu andar para trás 0 vezes
secao: o contador pareceu andar para trás 0 vezes
ring buffer: 50000 lidos, 0 fora de ordem, 0 descartados (buffer cheio)
...
ok: cortex-m0: atomic_fetch_add falha no link (__atomic_fetch_add_4)
ok: cortex-m0: ring buffer linka sem nenhuma __atomic_* (4 DMBs)
```

Os números de host variam a cada execução: as perdas do `volatile` dependem de onde o sinal
cai. No x86, o Clang faz o incremento de um `volatile` numa instrução só (`incl`), que um sinal
não divide, e o `volatile` não perde nada; o teste reconhece esse caso. No Cortex-M, que não
tem instrução que modifique a memória diretamente, o incremento são sempre três instruções.

## No QEMU (Zephyr)

Precisa do ambiente do Zephyr (west, `ZEPHYR_BASE` e o Zephyr SDK, que traz o QEMU), como no
[guia de primeiros passos](https://docs.zephyrproject.org/latest/develop/getting_started/index.html).

```sh
scripts/qemu.sh                    # qemu_cortex_m0 e qemu_cortex_m3, nos dois modos
scripts/qemu.sh qemu_cortex_m0     # uma placa só
```

O `QEMU_EXTRA_FLAGS` é lido pelo CMake na configuração do build, e não no `west build -t run`;
por isso o script usa um diretório de build por modo. Resultado (Zephyr 4.4.99, Zephyr SDK
1.0.1, QEMU 10.0.2):

```
== qemu_cortex_m0, modo padrao
  contador volatile: 2850 interrupções, esperado 50002850, obtido 50002850, perdidos 0
  contador atomic_t: 67736 interrupções, esperado 50067736, obtido 50067736, perdidos 0
  leitura de 64 bits: 4160 interrupções, contador andou para trás 0 vezes
  buffer do Neves: 200000 lidos, 0 fora de ordem, items = 0 (real = 0)
== qemu_cortex_m0, modo uma-instrucao
  contador volatile: 105313 interrupções, esperado 50105313, obtido 50082105, perdidos 23208
  contador atomic_t: 204446 interrupções, esperado 50204446, obtido 50204446, perdidos 0
  leitura de 64 bits: 100963 interrupções, contador andou para trás 3384 vezes
  buffer do Neves: 200002 lidos, 0 fora de ordem, items = 0 (real = 112)
== qemu_cortex_m3, modo padrao (idêntico no modo uma-instrucao)
  contador volatile: 352190 interrupções, esperado 50352190, obtido 50251566, perdidos 100624
  contador atomic_t: 606822 interrupções, esperado 50606822, obtido 50606822, perdidos 0
  leitura de 64 bits: 554564 interrupções, contador andou para trás 0 vezes
  buffer do Neves: 200001 lidos, 2160 fora de ordem, items = 0 (real = 105)
```

No modo padrão, o QEMU só atende interrupções entre blocos de instruções traduzidos, e um
`LDR`/`STR` no mesmo bloco nunca é interrompido: na `qemu_cortex_m0`, nada falha. Com uma
instrução por bloco, as falhas aparecem. A `qemu_cortex_m3` liga `-icount shift=6` na
configuração da placa, o que faz o timer disparar em um ponto exato da contagem de instruções,
mesmo no meio de um bloco: as falhas aparecem nos dois modos, com números idênticos a cada
execução. A leitura de 64 bits não rasga no M3 porque ali ela é um único `LDRD`.
