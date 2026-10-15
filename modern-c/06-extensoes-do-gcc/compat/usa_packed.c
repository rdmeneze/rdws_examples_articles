/* Com -DCOMPAT_SIMULA_OUTRO_COMPILADOR, deve FALHAR com a mensagem do PACKED, em vez de
 * compilar uma struct com padding sem ninguém perceber. */
#include "compiler.h"

struct PACKED cabecalho {
    uint8_t  tipo;
    uint32_t valor;
};

int tamanho(void)
{
    return (int)sizeof(struct cabecalho);
}
