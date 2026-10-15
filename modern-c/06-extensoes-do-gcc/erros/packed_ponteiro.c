/* Deve gerar AVISO (-Waddress-of-packed-member, ligado por padrão): o ponteiro pode ficar
 * desalinhado, e um acesso de 32 bits desalinhado gera falha em um Cortex-M0. */
#include <stdint.h>

struct __attribute__((packed)) mensagem {
    uint8_t  tipo;
    uint32_t valor;
};

uint32_t *ponteiro_para_valor(struct mensagem *m)
{
    return &m->valor;
}
