/* weak e alias: o padrão dos arquivos de startup. Todo handler aponta para
 * default_handler, a menos que a aplicação defina o seu (ver uart.c). */
#include <stdio.h>

void default_handler(void)
{
    puts("default_handler: interrupção sem tratador");
}

void uart_isr(void)  __attribute__((weak, alias("default_handler")));
void timer_isr(void) __attribute__((weak, alias("default_handler")));

int main(void)
{
    puts("uart_isr():");
    uart_isr();
    puts("timer_isr():");
    timer_isr();
    return 0;
}
