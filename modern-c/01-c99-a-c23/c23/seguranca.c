/* C23: aritmética com verificação de overflow (stdckdint.h, GCC 14+) e memset_explicit
 * (depende da libc; o Makefile define HAVE_MEMSET_EXPLICIT quando ela existe). */
#include <errno.h>
#include <stdint.h>
#include <stdio.h>

#if __has_include(<stdckdint.h>)
#define TEM_CKD 1
#include <stdckdint.h>
#endif
#include <string.h>

#if TEM_CKD

static int total_len(uint16_t header_len, uint16_t payload_len, uint16_t *out)
{
    uint16_t total;

    if (ckd_add(&total, header_len, payload_len)) {
        return -EOVERFLOW;
    }
    *out = total;
    return 0;
}
#endif

int main(void)
{
#if TEM_CKD
    uint16_t t;
    printf("total_len(8, 100)   -> %d\n", total_len(8, 100, &t));
    printf("total_len(8, 65530) -> %d (-EOVERFLOW = %d)\n", total_len(8, 65530, &t), -EOVERFLOW);
#else
    puts("ckd_add: stdckdint.h não disponível neste compilador");
#endif

    uint8_t psk[16];
    memset(psk, 0xa5, sizeof psk);   /* no firmware, a chave carregada da flash */
#ifdef HAVE_MEMSET_EXPLICIT
    memset_explicit(psk, 0, sizeof psk);
    printf("memset_explicit: psk[0]=%u psk[15]=%u\n", psk[0], psk[15]);
#else
    puts("memset_explicit: não disponível nesta libc");
#endif

    return 0;
}
