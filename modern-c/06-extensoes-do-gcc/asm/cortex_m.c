/* Inline assembly com instruções do Cortex-M. Só compila para ARM (o teste usa o GCC do
 * Zephyr SDK ou o indicado em ARM_CC). O clobber "memory" impede que o compilador mova
 * acessos à memória para fora da região protegida. */
static inline void irq_disable(void)
{
    __asm__ volatile("cpsid i" ::: "memory");
}

static inline void irq_enable(void)
{
    __asm__ volatile("cpsie i" ::: "memory");
}

static inline void wait_for_interrupt(void)
{
    __asm__ volatile("wfi");
}

volatile unsigned contador;

void incrementa_protegido(void)
{
    irq_disable();
    contador++;
    irq_enable();
    wait_for_interrupt();
}
