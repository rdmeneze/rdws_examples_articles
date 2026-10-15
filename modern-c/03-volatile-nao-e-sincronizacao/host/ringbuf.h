/* Ring buffer de um produtor (a ISR) e um consumidor (o código principal), sem travas.
 * Cada índice é escrito por um lado só; acquire/release garantem que o dado escrito no
 * buffer fica visível antes do índice que o publica. */
#ifndef RINGBUF_H
#define RINGBUF_H

#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>

#define RB_TAMANHO 64u   /* potência de 2 */

typedef struct {
    uint32_t dados[RB_TAMANHO];
    atomic_uint cabeca;   /* escrito só pelo produtor */
    atomic_uint cauda;    /* escrito só pelo consumidor */
} ringbuf_t;

/* Chamado pela ISR. */
static inline bool rb_poe(ringbuf_t *rb, uint32_t v)
{
    unsigned cab = atomic_load_explicit(&rb->cabeca, memory_order_relaxed);
    unsigned cau = atomic_load_explicit(&rb->cauda, memory_order_acquire);

    if (cab - cau == RB_TAMANHO) {
        return false;                                   /* cheio */
    }
    rb->dados[cab % RB_TAMANHO] = v;
    atomic_store_explicit(&rb->cabeca, cab + 1, memory_order_release);
    return true;
}

/* Chamado pelo código principal. */
static inline bool rb_tira(ringbuf_t *rb, uint32_t *v)
{
    unsigned cau = atomic_load_explicit(&rb->cauda, memory_order_relaxed);
    unsigned cab = atomic_load_explicit(&rb->cabeca, memory_order_acquire);

    if (cab == cau) {
        return false;                                   /* vazio */
    }
    *v = rb->dados[cau % RB_TAMANHO];
    atomic_store_explicit(&rb->cauda, cau + 1, memory_order_release);
    return true;
}

#endif
