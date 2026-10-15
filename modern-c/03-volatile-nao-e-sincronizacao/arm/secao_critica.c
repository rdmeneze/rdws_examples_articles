/* Seção crítica no Cortex-M: salva o PRIMASK, desliga as interrupções e restaura o estado
 * anterior (e não simplesmente religa, para funcionar mesmo se já estavam desligadas).
 * O CMSIS oferece __get_PRIMASK, __disable_irq e __set_PRIMASK com o mesmo efeito. */
#include <stdint.h>

volatile uint64_t ticks;

static inline uint32_t irq_salva_e_desliga(void)
{
    uint32_t primask;

    __asm__ volatile("mrs %0, primask\n\tcpsid i" : "=r"(primask) : : "memory");
    return primask;
}

static inline void irq_restaura(uint32_t primask)
{
    __asm__ volatile("msr primask, %0" : : "r"(primask) : "memory");
}

uint64_t le_ticks(void)
{
    uint32_t chave = irq_salva_e_desliga();
    uint64_t v = ticks;

    irq_restaura(chave);
    return v;
}
