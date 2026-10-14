/* C11: atomics. O padrão garante atomicidade, não que a operação seja lock-free nem
 * adequada para uma ISR. No host, uart_isr() é chamada direto; o alvo `arm` (etapa 3)
 * compila este arquivo para Cortex-M0 e M4. */
#include <stdatomic.h>
#include <stdio.h>

/* SEM_ASSERT_LOCK_FREE só existe para o teste ver o código gerado no Cortex-M0. */
#ifndef SEM_ASSERT_LOCK_FREE
_Static_assert(ATOMIC_INT_LOCK_FREE == 2,
               "atomic_int precisa ser lock-free para uso em ISR");
#endif

static atomic_uint rx_count;

void uart_isr(void)
{
    atomic_fetch_add(&rx_count, 1);
}

int main(void)
{
    for (int i = 0; i < 5; i++) {
        uart_isr();
    }

    printf("rx_count = %u\n", atomic_load(&rx_count));
    printf("ATOMIC_INT_LOCK_FREE = %d (0 nunca, 1 às vezes, 2 sempre)\n", ATOMIC_INT_LOCK_FREE);
    printf("atomic_is_lock_free(&rx_count) = %d\n", (int)atomic_is_lock_free(&rx_count));
    return 0;
}
