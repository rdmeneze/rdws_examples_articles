/* Statement expressions e __auto_type: um MIN que avalia cada argumento uma vez só. */
#include <stdio.h>

#define MIN_INGENUO(a, b) ((a) < (b) ? (a) : (b))

#define MIN(a, b) ({            \
    __auto_type a_ = (a);       \
    __auto_type b_ = (b);       \
    a_ < b_ ? a_ : b_;          \
})

static int chamadas;
static int le_adc(void) { chamadas++; return 512; }

int main(void)
{
    chamadas = 0;
    int x = MIN_INGENUO(le_adc(), 1000);
    printf("MIN_INGENUO = %d, le_adc() chamada %d vezes\n", x, chamadas);

    chamadas = 0;
    int y = MIN(le_adc(), 1000);
    printf("MIN         = %d, le_adc() chamada %d vez\n", y, chamadas);
    return 0;
}
