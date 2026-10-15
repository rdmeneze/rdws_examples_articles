/* Leitura-modificação-escrita: o código principal e a "ISR" incrementam o mesmo contador.
 * Uso: rmw volatile | rmw atomic */
#define _POSIX_C_SOURCE 200809L
#include <stdatomic.h>
#include <stdio.h>
#include <string.h>

#include "isr.h"

static volatile unsigned contador_v;
static atomic_uint contador_a;
static volatile sig_atomic_t usa_atomic, isr_n;

static void isr(int s)
{
    (void)s;
    if (usa_atomic) {
        atomic_fetch_add(&contador_a, 1);
    } else {
        contador_v++;
    }
    isr_n++;
}

int main(int argc, char **argv)
{
    const unsigned n = 300000000u;

    usa_atomic = argc > 1 && strcmp(argv[1], "atomic") == 0;
    isr_liga(isr);
    for (unsigned i = 0; i < n; i++) {
        if (usa_atomic) {
            atomic_fetch_add(&contador_a, 1);
        } else {
            contador_v++;
        }
    }
    isr_desliga();

    unsigned esperado = n + (unsigned)isr_n;
    unsigned obtido = usa_atomic ? atomic_load(&contador_a) : contador_v;
    printf("%s: esperado %u, obtido %u, incrementos perdidos: %u\n",
           usa_atomic ? "atomic" : "volatile", esperado, obtido, esperado - obtido);
    return 0;
}
