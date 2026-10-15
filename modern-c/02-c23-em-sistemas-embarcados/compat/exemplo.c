/* Usa compat_c23.h com qualquer padrão de C11 a C23. */
#include <stdint.h>
#include <stdio.h>

#include "compat_c23.h"

static_assert(sizeof(uint32_t) * 8 == 32, "uint32_t com 32 bits");

int main(void)
{
    bool ok = true;
    int *p = nullptr;
    uint16_t total;
    uint8_t chave[16] = { 1, 2, 3 };

    if (ckd_add(&total, (uint16_t)65000, (uint16_t)1000)) {
        puts("ckd_add: overflow detectado");
    }
    memset_explicit(chave, 0, sizeof chave);

    printf("C23 nativo: %s, stdckdint.h: %s, chave[0] = %u, p %s\n",
           COMPAT_C23 ? "sim" : "não",
#ifdef COMPAT_TEM_CKDINT
           "sim",
#else
           "não (builtin do GCC)",
#endif
           chave[0], p == nullptr ? "nulo" : "?");
    return ok ? 0 : 1;
}
