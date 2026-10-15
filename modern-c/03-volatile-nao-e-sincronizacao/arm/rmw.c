/* flags |= BIT num volatile: três instruções (LDR, ORR, STR). Uma ISR que ligue outro bit
 * entre o LDR e o STR tem o bit apagado pelo STR. */
#include <stdint.h>

volatile uint32_t flags;

void liga_bit(void)
{
    flags |= 0x4;
}
