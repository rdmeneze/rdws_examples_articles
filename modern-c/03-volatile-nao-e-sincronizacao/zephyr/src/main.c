/* As armadilhas do artigo em um Cortex-M de verdade (emulado pelo QEMU), com uma interrupção
 * de verdade: a expiração de um k_timer roda no contexto da ISR do relógio do sistema. */
#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/atomic.h>

#include "buffer_circ.h"

enum experimento { NENHUM, CONTADOR_VOLATILE, CONTADOR_ATOMIC, TICKS64, NEVES };

static volatile enum experimento atual;
static volatile uint32_t isr_n;

static volatile uint32_t contador_v;
static atomic_t contador_a;
static volatile uint64_t ticks64;
static CIRCULAR_BUFFER_DECLARE(buf);
static unsigned char proximo;

static void isr(struct k_timer *t)
{
    ARG_UNUSED(t);
    switch (atual) {
    case CONTADOR_VOLATILE:
        contador_v++;
        break;
    case CONTADOR_ATOMIC:
        atomic_inc(&contador_a);
        break;
    case TICKS64:
        ticks64 += 0x80000000u;   /* a parte baixa vira a cada duas interrupções */
        break;
    case NEVES: {
        unsigned char v = proximo;

        if (buffer_insert(&buf, &v, 1) == 0) {
            proximo++;
        }
        break;
    }
    default:
        return;
    }
    isr_n++;
}

K_TIMER_DEFINE(timer_isr, isr, NULL);

#define N 50000000u

static void contador(enum experimento e)
{
    isr_n = 0;
    atual = e;
    for (uint32_t i = 0; i < N; i++) {
        if (e == CONTADOR_VOLATILE) {
            contador_v++;
        } else {
            atomic_inc(&contador_a);
        }
    }
    atual = NENHUM;

    uint32_t esperado = N + isr_n;
    uint32_t obtido = e == CONTADOR_VOLATILE ? contador_v : (uint32_t)atomic_get(&contador_a);

    printk("%s: %u interrupções, esperado %u, obtido %u, perdidos %u\n",
           e == CONTADOR_VOLATILE ? "contador volatile" : "contador atomic_t",
           isr_n, esperado, obtido, esperado - obtido);
}

static void leitura_64(void)
{
    uint64_t anterior = 0;
    uint32_t recuos = 0;

    isr_n = 0;
    atual = TICKS64;
    for (uint32_t i = 0; i < N; i++) {
        uint64_t v = ticks64;

        recuos += v < anterior;
        anterior = v;
    }
    atual = NENHUM;
    printk("leitura de 64 bits: %u interrupções, contador andou para trás %u vezes\n", isr_n, recuos);
}

static void buffer_neves(void)
{
    unsigned char esperado = 0, v;
    uint32_t lidos = 0, fora_de_ordem = 0;

    isr_n = 0;
    atual = NEVES;
    while (lidos < 200000u) {
        for (volatile int t = 0; t < 2000; t++) {
        }
        while (buffer_retrieve(&buf, &v, 1) == 1) {
            fora_de_ordem += v != esperado;
            esperado = (unsigned char)(v + 1);
            lidos++;
        }
    }
    atual = NENHUM;

    unsigned real = (buf.wr_ptr - buf.rd_ptr + BUFFER_SIZE) % BUFFER_SIZE;

    printk("buffer_FN: %u lidos, %u fora de ordem, items = %u (real = %u)\n",
           lidos, fora_de_ordem, buf.items, real);
}

int main(void)
{
    printk("=== %s\n", CONFIG_BOARD_TARGET);
    k_timer_start(&timer_isr, K_TICKS(1), K_TICKS(1));
    contador(CONTADOR_VOLATILE);
    contador(CONTADOR_ATOMIC);
    leitura_64();
    buffer_neves();
    k_timer_stop(&timer_isr);
    printk("=== FIM\n");
    return 0;
}
