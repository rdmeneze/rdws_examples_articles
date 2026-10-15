/* format(printf, ...): o compilador passa a conferir as chamadas da função de log do
 * projeto como confere as do printf. O erro aparece em erros/format_errado.c. */
#include <stdarg.h>
#include <stdio.h>

__attribute__((format(printf, 2, 3)))
static void log_msg(int nivel, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    printf("[%d] ", nivel);
    vprintf(fmt, ap);
    va_end(ap);
}

int main(void)
{
    log_msg(1, "temperatura = %d.%d C\n", 21, 5);
    log_msg(2, "sensor %s sem resposta\n", "sht31");
    return 0;
}
