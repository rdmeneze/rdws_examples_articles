/* C99: restrict. A promessa vale para os acessos durante a chamada: o que é escrito por
 * dst só é acessado por dst (ou por valores derivados dele, como dst + i). */
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

void copy(uint8_t *restrict dst, const uint8_t *restrict src, size_t n);

void copy(uint8_t *restrict dst, const uint8_t *restrict src, size_t n)
{
    for (size_t i = 0; i < n; i++) {
        dst[i] = src[i];
    }
}

int main(void)
{
    const uint8_t origem[4] = { 1, 2, 3, 4 };
    uint8_t destino[4];

    copy(destino, origem, sizeof origem);   /* regiões distintas: ok */
    printf("destino: %u %u %u %u\n", destino[0], destino[1], destino[2], destino[3]);

    /* copy(destino + 1, destino, 3) quebraria a promessa: comportamento indefinido. */
    return 0;
}
