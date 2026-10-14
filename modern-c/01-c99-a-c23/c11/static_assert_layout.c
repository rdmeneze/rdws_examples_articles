/* C11: _Static_assert protege as premissas de layout deste build (compilador e
 * arquitetura). Não torna o layout portável. */
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef struct {
    uint8_t  id;
    uint8_t  len;
    uint16_t crc;
    uint32_t timestamp;
} packet_header_t;

_Static_assert(sizeof(packet_header_t) == 8,
               "layout do cabeçalho mudou");
_Static_assert(offsetof(packet_header_t, timestamp) == 4,
               "timestamp fora da posição esperada");

int main(void)
{
    printf("sizeof(packet_header_t) = %zu\n", sizeof(packet_header_t));
    printf("offsetof(crc) = %zu, offsetof(timestamp) = %zu\n",
           offsetof(packet_header_t, crc), offsetof(packet_header_t, timestamp));
    return 0;
}
