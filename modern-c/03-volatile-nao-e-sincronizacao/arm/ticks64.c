/* Contador de 64 bits incrementado por uma ISR. No Cortex-M0, a leitura são dois LDR; no M4,
 * um LDRD, que o próprio GCC não considera atômico: atomic_load de 64 bits vira uma chamada
 * a __atomic_load_8. */
#include <stdatomic.h>
#include <stdint.h>

volatile uint64_t ticks;
atomic_ullong ticks_atomic;

uint64_t le_ticks(void)
{
    return ticks;
}

uint64_t le_ticks_atomic(void)
{
    return atomic_load(&ticks_atomic);
}
