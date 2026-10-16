#include <stddef.h>
#include <stdint.h>

[[nodiscard("verifique o código de erro")]]
int flash_write(uint32_t addr, const void *data, size_t len);

void salva_config(const void *cfg, size_t len)
{
    flash_write(0x0800'0000, cfg, len);   /* retorno ignorado */
}
