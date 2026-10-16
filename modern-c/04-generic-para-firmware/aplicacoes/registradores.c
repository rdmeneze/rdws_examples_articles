/* Escrita em registrador escolhida pela largura do ponteiro. Sem default: um ponteiro de outro
 * tipo não compila (veja -DTIPO_ERRADO). */
#include <stdint.h>

static inline void reg_wr8 (volatile uint8_t  *r, uint8_t  v) { *r = v; }
static inline void reg_wr16(volatile uint16_t *r, uint16_t v) { *r = v; }
static inline void reg_wr32(volatile uint32_t *r, uint32_t v) { *r = v; }

#define REG_WRITE(r, v) _Generic((r), \
    volatile uint8_t *:  reg_wr8,     \
    volatile uint16_t *: reg_wr16,    \
    volatile uint32_t *: reg_wr32)((r), (v))

typedef struct {
    volatile uint32_t CR;
    volatile uint16_t BRR;
    volatile uint8_t  DR;
} uart_t;

#define UART ((uart_t *)0x40004400u)

void uart_configura(void)
{
    REG_WRITE(&UART->CR, 0x200C);
    REG_WRITE(&UART->BRR, 0x1A1);
    REG_WRITE(&UART->DR, 'A');
}

#ifdef TIPO_ERRADO
void erro(void)
{
    volatile int *p = (volatile int *)0x40000000u;
    REG_WRITE(p, 1);
}
#endif
