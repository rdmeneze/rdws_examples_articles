/* Deve gerar AVISO: o retorno de flash_write é ignorado. O teste usa -Werror=unused-result. */
#include <stddef.h>
#include <stdint.h>

[[nodiscard("verifique o código de erro")]]
int flash_write(uint32_t addr, const void *data, size_t len);

int flash_write(uint32_t addr, const void *data, size_t len)
{
    (void)addr; (void)data;
    return len > 0 ? 0 : -1;
}

int main(void)
{
    const uint8_t dados[4] = { 0 };

    flash_write(0x0800'0000, dados, sizeof dados);   /* erro de escrita que ninguém verifica */
    return 0;
}
