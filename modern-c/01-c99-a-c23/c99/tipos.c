/* C99: stdint.h e inttypes.h. Cada tipo tem sua própria macro de formato. */
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

int main(void)
{
    uint32_t      timestamp = 1760400000u;
    uint_least8_t canal     = 3;
    uint_fast8_t  tentativa = 2;

    printf("uint32_t      = %" PRIu32 ", sizeof = %zu\n", timestamp, sizeof timestamp);
    printf("uint_least8_t = %" PRIuLEAST8 ", sizeof = %zu\n", canal, sizeof canal);
    printf("uint_fast8_t  = %" PRIuFAST8 ", sizeof = %zu nesta plataforma\n", tentativa, sizeof tentativa);

    return 0;
}
