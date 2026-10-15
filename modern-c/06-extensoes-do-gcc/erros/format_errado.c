/* Deve FALHAR com -Werror=format: %d recebendo uma string. */
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
    log_msg(1, "temperatura = %d C\n", "21");
    return 0;
}
