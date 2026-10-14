/* C23: constexpr, static_assert sem mensagem, typeof, auto e inicializador vazio. */
#include <stdint.h>
#include <stdio.h>

typedef struct {
    uint8_t  id;
    uint8_t  len;
    uint16_t crc;
    uint32_t timestamp;
} packet_header_t;

constexpr uint32_t SYSTICK_HZ     = 1'000;
constexpr uint32_t SYSTICK_RELOAD = 480'000'000 / SYSTICK_HZ - 1;

static_assert(SYSTICK_RELOAD <= 0xFF'FFFF);   /* o SysTick tem 24 bits */

#define SWAP(a, b) do { typeof(a) tmp_ = (a); (a) = (b); (b) = tmp_; } while (0)

#define TIPO(x) _Generic((x), int: "int", unsigned int: "unsigned int", default: "outro")

int main(void)
{
    auto timeout = 1000u;          /* unsigned int */
    packet_header_t hdr = {};      /* zera o objeto inteiro */

    int a = 1, b = 2;
    SWAP(a, b);

    printf("SYSTICK_RELOAD = %lu\n", (unsigned long)SYSTICK_RELOAD);
    printf("SWAP: a=%d b=%d\n", a, b);
    printf("auto timeout = 1000u -> %s\n", TIPO(timeout));
    printf("hdr = {}: id=%u len=%u crc=%u timestamp=%lu\n",
           hdr.id, hdr.len, hdr.crc, (unsigned long)hdr.timestamp);
    return 0;
}
