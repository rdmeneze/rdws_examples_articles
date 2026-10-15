/* O que cada ordem de memória gera no Cortex-M. */
#include <stdatomic.h>

atomic_uint indice;

void st_relaxed(unsigned v) { atomic_store_explicit(&indice, v, memory_order_relaxed); }
void st_release(unsigned v) { atomic_store_explicit(&indice, v, memory_order_release); }
void st_seq_cst(unsigned v) { atomic_store(&indice, v); }

unsigned ld_relaxed(void)   { return atomic_load_explicit(&indice, memory_order_relaxed); }
unsigned ld_acquire(void)   { return atomic_load_explicit(&indice, memory_order_acquire); }
unsigned ld_seq_cst(void)   { return atomic_load(&indice); }
