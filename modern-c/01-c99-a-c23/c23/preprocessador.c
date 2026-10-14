/* C23: __VA_OPT__ e #embed. O #embed exige GCC 15 ou Clang 19; sem ele, o exemplo avisa. */
#include <stdint.h>
#include <stdio.h>

#if defined(__has_embed)
#define TEM_EMBED 1
#else
#define TEM_EMBED 0
#endif

int main(void)
{
    int x = 42;

#define LOG(fmt, ...) printf(fmt __VA_OPT__(,) __VA_ARGS__)

    LOG("iniciando\n");       /* ok: printf("iniciando\n") */
    LOG("valor = %d\n", x);   /* ok: printf("valor = %d\n", x) */

#if TEM_EMBED
    static const uint8_t ca_cert[] = {
#embed "certs/ca.der"
    };
    printf("ca_cert: %zu bytes, começa com 0x%02x 0x%02x (sequência DER)\n",
           sizeof ca_cert, ca_cert[0], ca_cert[1]);
#else
    puts("#embed: não suportado por este compilador");
#endif

    return 0;
}
