/* A "ISR" produz uma sequência no ring buffer; o código principal consome e confere a ordem. */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>

#include "isr.h"
#include "ringbuf.h"

static ringbuf_t rb;
static uint32_t proximo;
static volatile sig_atomic_t descartados;

static void isr(int s)
{
    (void)s;
    if (rb_poe(&rb, proximo)) {
        proximo++;
    } else {
        descartados++;
    }
}

int main(void)
{
    uint32_t esperado = 0, v;
    unsigned fora_de_ordem = 0, lidos = 0;

    isr_liga(isr);
    while (lidos < 50000) {
        if (rb_tira(&rb, &v)) {
            fora_de_ordem += v != esperado;
            esperado = v + 1;
            lidos++;
        }
    }
    isr_desliga();
    printf("ring buffer: %u lidos, %u fora de ordem, %d descartados (buffer cheio)\n",
           lidos, fora_de_ordem, (int)descartados);
    return fora_de_ordem != 0;
}
