/* Usa compiler.h. Com -DCOMPAT_SIMULA_OUTRO_COMPILADOR, finge que o compilador não tem
 * __has_attribute nem __has_builtin: PRINTF_FMT e likely somem, BSWAP32 usa o substituto. */
#include <stdarg.h>
#include <stdio.h>

#include "compiler.h"

PRINTF_FMT(1, 2)
static void log_msg(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
}

int main(void)
{
    uint32_t v = 0x12345678;

    if (likely(v != 0)) {
        log_msg("BSWAP32(0x%08lx) = 0x%08lx (%s)\n", (unsigned long)v, (unsigned long)BSWAP32(v),
#if TEM_BUILTIN(__builtin_bswap32)
                "builtin"
#else
                "substituto portável"
#endif
        );
    }
    return 0;
}
