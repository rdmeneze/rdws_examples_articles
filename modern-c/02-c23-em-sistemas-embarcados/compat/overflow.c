/* A abertura do artigo: a mesma soma com detecção de overflow, primeiro com o builtin do GCC
 * e depois com o ckd_add do C23. A segunda só compila onde houver stdckdint.h. */
#include <errno.h>
#include <stdint.h>

int soma_gnu(uint32_t a, uint32_t b, uint32_t *saida)
{
    uint32_t result;

    if (__builtin_add_overflow(a, b, &result)) {
        return -ERANGE;
    }
    *saida = result;
    return 0;
}

#if defined(__has_include)
#if __has_include(<stdckdint.h>)
#include <stdckdint.h>

int soma_c23(uint32_t a, uint32_t b, uint32_t *saida)
{
    uint32_t result;

    if (ckd_add(&result, a, b)) {
        return -ERANGE;
    }
    *saida = result;
    return 0;
}
#endif
#endif
