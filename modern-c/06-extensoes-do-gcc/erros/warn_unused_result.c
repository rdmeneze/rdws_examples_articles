/* Deve gerar AVISO: o retorno de flash_write é ignorado. Antecessor do [[nodiscard]]. */
#include <stddef.h>

__attribute__((warn_unused_result))
int flash_write(unsigned addr, const void *data, size_t len);

void salva(const void *cfg, size_t len)
{
    flash_write(0x08000000u, cfg, len);
}
