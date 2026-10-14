/* C11: alinhamento explícito, por exemplo para buffers de DMA alinhados à linha de cache
 * de 32 bytes do Cortex-M7. */
#include <stdalign.h>
#include <stdint.h>
#include <stdio.h>

static alignas(32) uint8_t dma_buffer[512];

int main(void)
{
    printf("alignof(uint32_t) = %zu\n", alignof(uint32_t));
    printf("dma_buffer em %p, endereço %% 32 = %lu\n",
           (void *)dma_buffer, (unsigned long)((uintptr_t)dma_buffer % 32));
    return 0;
}
