/* Usar ATOMIC_INT_LOCK_FREE para decidir, e não só para barrar o build: onde o atomic_uint é
 * sempre lock-free, um atomic_fetch_add; onde não é (Cortex-M0), uma seção crítica. Quem chama
 * conta_rx() não precisa saber qual das duas foi escolhida. */
#include <stdatomic.h>
#include <stdint.h>

#if ATOMIC_INT_LOCK_FREE == 2

static atomic_uint rx_count;

static inline void conta_rx(void)
{
    atomic_fetch_add_explicit(&rx_count, 1, memory_order_relaxed);
}

#else

static volatile unsigned rx_count;

static inline void conta_rx(void)
{
    uint32_t primask;

    __asm__ volatile("mrs %0, primask\n\tcpsid i" : "=r"(primask) : : "memory");
    rx_count++;
    __asm__ volatile("msr primask, %0" : : "r"(primask) : "memory");
}

#endif

void uart_isr(void)
{
    conta_rx();
}

void tarefa(void)
{
    conta_rx();
}

int main(void)
{
    tarefa();
    return 0;
}
