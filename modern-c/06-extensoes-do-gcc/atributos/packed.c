/* aligned e packed. packed remove o padding: útil para espelhar um formato de protocolo,
 * perigoso quando se pega o endereço de um membro (ver erros/packed_ponteiro.c). */
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

struct normal {
    uint8_t  tipo;
    uint32_t valor;
};

struct __attribute__((packed)) empacotada {
    uint8_t  tipo;
    uint32_t valor;
};

static uint8_t dma_buffer[64] __attribute__((aligned(32)));

int main(void)
{
    printf("sizeof(struct normal) = %zu, offsetof(valor) = %zu\n",
           sizeof(struct normal), offsetof(struct normal, valor));
    printf("sizeof(struct empacotada) = %zu, offsetof(valor) = %zu\n",
           sizeof(struct empacotada), offsetof(struct empacotada, valor));
    printf("dma_buffer %% 32 = %lu\n", (unsigned long)((uintptr_t)dma_buffer % 32));
    return 0;
}
