/* Array de tamanho zero: extensão antiga do GCC. Com -pedantic-errors, deve FALHAR; o
 * equivalente padrão é o flexible array member (uint8_t payload[]). */
#include <stdint.h>

struct pacote {
    uint8_t len;
    uint8_t payload[0];
};

int main(void)
{
    return (int)sizeof(struct pacote) - 1;
}
