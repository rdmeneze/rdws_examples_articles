/* C99: macros variádicas. A chamada sem argumentos extras, que não compila, está em
 * erros/log_sem_argumentos.c; a solução do C23 está em c23/. */
#include <stdio.h>

#define LOG(fmt, ...) \
    printf(fmt, __VA_ARGS__)

int main(void)
{
    int x = 42;

    LOG("valor = %d\n", x);   /* ok */
    LOG("%s: %d leituras\n", "sensor", 3);

    return 0;
}
