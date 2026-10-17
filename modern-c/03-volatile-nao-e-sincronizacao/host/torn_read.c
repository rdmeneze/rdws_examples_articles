/* Torn read (leitura feita em pedaços) de um contador de 64 bits num processador de 32 bits,
 * emulada no host com duas metades lidas uma de cada vez (como faz o Cortex-M0).
 * Uso: torn_read ingenua | torn_read repete | torn_read secao */
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "isr.h"

static volatile uint32_t ticks_lo, ticks_hi;

static void isr(int s)
{
    (void)s;
    uint32_t lo = ticks_lo + 0x80000000u;   /* passo grande: a parte baixa vira a cada duas ISRs */

    if (lo < ticks_lo) {
        ticks_hi++;
    }
    ticks_lo = lo;
}

static uint64_t le_ingenua(void)
{
    uint32_t hi = ticks_hi;
    uint32_t lo = ticks_lo;

    return ((uint64_t)hi << 32) | lo;
}

/* Lê de novo até a parte alta não mudar no meio. Vale quando só a ISR escreve. */
static uint64_t le_repete(void)
{
    uint32_t hi, lo;

    do {
        hi = ticks_hi;
        lo = ticks_lo;
    } while (hi != ticks_hi);
    return ((uint64_t)hi << 32) | lo;
}

/* Seção crítica: no firmware, desligar as interrupções; aqui, bloquear o sinal. */
static uint64_t le_secao(void)
{
    sigset_t antes;

    irq_desliga(&antes);
    uint64_t v = le_ingenua();
    irq_restaura(&antes);
    return v;
}

int main(int argc, char **argv)
{
    const char *modo = argc > 1 ? argv[1] : "ingenua";
    uint64_t (*le)(void) = strcmp(modo, "repete") == 0 ? le_repete
                         : strcmp(modo, "secao") == 0  ? le_secao : le_ingenua;
    unsigned n = strcmp(modo, "secao") == 0 ? 2000000u : 200000000u;   /* sigprocmask é lento */
    uint64_t anterior = 0, recuos = 0;

    isr_liga(isr);
    for (unsigned i = 0; i < n; i++) {
        uint64_t v = le();

        if (v < anterior) {
            recuos++;
        }
        anterior = v;
    }
    isr_desliga();
    printf("%s: o contador pareceu andar para trás %llu vezes\n", modo, (unsigned long long)recuos);
    return 0;
}
