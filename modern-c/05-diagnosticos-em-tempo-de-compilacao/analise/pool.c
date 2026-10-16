#include <stddef.h>
#include <string.h>

void pool_free(void *p);
__attribute__((malloc, malloc(pool_free, 1)))
void *pool_alloc(size_t n);

int envia(const char *msg, size_t n);

int monta_e_envia(const char *texto)
{
    char *buf = pool_alloc(64);

    if (buf == NULL) {
        return -1;
    }
    strncpy(buf, texto, 63);
    buf[63] = '\0';
    if (envia(buf, strlen(buf)) < 0) {
        return -2;                /* esqueceu o pool_free: o bloco vaza */
    }
    pool_free(buf);
    return 0;
}
