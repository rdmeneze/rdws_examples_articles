/* Definição "forte": quando este arquivo entra no link, substitui o alias fraco. */
#include <stdio.h>

void uart_isr(void)
{
    puts("uart_isr da aplicação");
}
