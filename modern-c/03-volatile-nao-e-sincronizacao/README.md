# Exemplos: volatile não é sincronização

Acompanha o artigo "volatile não é sincronização: usando atomics do C11 em firmware" (link do
Embarcados quando for publicado).

| Pasta | O que tem |
|---|---|
| `host/` | Programas que rodam no Linux, com um sinal `SIGALRM` a cada 20 µs fazendo o papel da ISR (`isr.h`): incrementos perdidos com `volatile` e sem perda com `atomic` (`rmw.c`), leitura rasgada de um contador de 64 bits e duas correções (`rasgada.c`), ring buffer de um produtor e um consumidor com acquire/release (`ringbuf.h`, `ring.c`); o buffer circular de Felipe Neves (Embarcados, 2016) entre a "ISR" e o código principal (`neves.c`) |
| `arm/` | Arquivos que só compilam para Cortex-M; o teste confere o assembly: `flags \|= BIT` (`rmw.c`), leitura de 64 bits (`ticks64.c`), reordenação em volta de um `volatile` (`reordena.c`), DMBs por ordem de memória (`ordens.c`), `atomic_fetch_add` no M0 (`fetch_add.c`), o ring buffer no M0 (`ring.c`) e seção crítica com PRIMASK (`secao_critica.c`) |
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
