/* Deve FALHAR com -std=c99: sem argumentos extras, sobra uma vírgula antes do ")".
 * A solução do C23 (__VA_OPT__) está em c23/preprocessador.c. */
#include <stdio.h>

#define LOG(fmt, ...) \
    printf(fmt, __VA_ARGS__)

int main(void)
{
    int x = 42;

    LOG("valor = %d\n", x);   /* ok */
    LOG("iniciando\n");       /* erro: vira printf("iniciando\n", ) */

    return 0;
}
