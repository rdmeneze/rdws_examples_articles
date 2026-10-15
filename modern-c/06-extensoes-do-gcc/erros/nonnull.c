/* Deve gerar AVISO com -Wall: argumento nulo para parâmetro marcado como nonnull. */
#include <stddef.h>

__attribute__((nonnull(1)))
int envia(const char *dados, size_t len);

int main(void)
{
    return envia(NULL, 0);
}
