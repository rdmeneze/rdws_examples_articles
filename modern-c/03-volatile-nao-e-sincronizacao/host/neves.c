/* Usa o buffer circular de Felipe Neves (baixado por scripts/baixa_neves.sh) entre uma
 * "ISR" que insere e o código principal que retira. O buffer mantém um contador de itens
 * atualizado pelos dois lados (items++ em buffer_insert, items-- em buffer_retrieve).
 *
 * O consumidor faz "outro trabalho" e esvazia o buffer em rajadas, como em firmware real.
 * A corrida existe, mas a janela é de uma instrução: espere não ver nenhuma falha. É esse o
 * ponto do exemplo. */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>

#include "isr.h"
#include "buffer_circ.h"

static CIRCULAR_BUFFER_DECLARE(buf);
static unsigned char proximo;
static volatile sig_atomic_t descartados;

static void isr(int s)
{
    unsigned char v = proximo;

    (void)s;
    if (buffer_insert(&buf, &v, 1) == 0) {
        proximo++;
    } else {
        descartados++;
    }
}

int main(int argc, char **argv)
{
    unsigned long alvo = argc > 1 ? strtoul(argv[1], NULL, 10) : 300000ul;
    unsigned char esperado = 0, v;
    unsigned long lidos = 0, fora_de_ordem = 0;

    isr_liga(isr);
    while (lidos < alvo) {
        for (volatile int t = 0; t < 20000; t++) {
        }
        while (buffer_retrieve(&buf, &v, 1) == 1) {
            fora_de_ordem += v != esperado;
            esperado = (unsigned char)(v + 1);
            lidos++;
        }
    }
    isr_desliga();

    unsigned real = (buf.wr_ptr - buf.rd_ptr + BUFFER_SIZE) % BUFFER_SIZE;
    printf("buffer do Neves: %lu lidos, %lu fora de ordem, items = %u (real = %u), %d descartados\n",
           lidos, fora_de_ordem, buf.items, real, (int)descartados);
    return 0;
}
