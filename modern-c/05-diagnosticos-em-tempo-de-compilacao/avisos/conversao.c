#include <stdint.h>
uint16_t media(const uint32_t *v, unsigned n)
{
    uint32_t soma = 0;
    for (unsigned i = 0; i < n; i++) soma += v[i];
    return soma / n;              /* uint32_t -> uint16_t, sem aviso no -Wall */
}
int atingiu(int lido, unsigned limite)
{
    return lido >= limite;        /* -1 >= 10u é verdadeiro */
}
