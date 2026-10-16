/* Listar int e int32_t juntos: erro no x86 (são o mesmo tipo), compila no ARM (int32_t é
 * long). E um _Generic que só conhece int e unsigned int manda o uint32_t para o default no
 * ARM. */
#include <stdint.h>

int lista_os_dois(int32_t x)
{
    return _Generic(x, int: 1, int32_t: 2, default: 3);
}

#define SO_INT(x) _Generic((x), int: "int", unsigned int: "unsigned int", default: "default")

const char *qual(void)
{
    uint32_t u = 0;
    return SO_INT(u);
}
