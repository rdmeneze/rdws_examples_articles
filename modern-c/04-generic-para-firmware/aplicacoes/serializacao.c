/* Grava um inteiro em big-endian escolhendo a largura pelo tipo. Um literal (int) não compila:
 * o tipo obriga a dizer o tamanho do campo (veja -DLITERAL). */
#include <stdint.h>
#include <stdio.h>

static inline void put_be16(uint8_t *p, uint16_t v) { v = __builtin_bswap16(v); __builtin_memcpy(p, &v, 2); }
static inline void put_be32(uint8_t *p, uint32_t v) { v = __builtin_bswap32(v); __builtin_memcpy(p, &v, 4); }
static inline void put_be64(uint8_t *p, uint64_t v) { v = __builtin_bswap64(v); __builtin_memcpy(p, &v, 8); }

#define PUT_BE(p, v) _Generic((v), \
    uint16_t: put_be16,            \
    uint32_t: put_be32,            \
    uint64_t: put_be64)((p), (v))

void serializa(uint8_t *buf, uint16_t id, uint32_t ts)
{
    PUT_BE(buf, id);
    PUT_BE(buf + 2, ts);
#ifdef LITERAL
    PUT_BE(buf + 6, 0x1234);
#endif
}

int main(void)
{
    uint8_t buf[6];

    serializa(buf, 0x0102, 0x03040506);
    for (unsigned i = 0; i < sizeof buf; i++) {
        printf("%02x ", buf[i]);
    }
    printf("\n");
    return 0;
}
