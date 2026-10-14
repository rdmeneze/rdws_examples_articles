/* C23: atributos padronizados. O aviso de [[nodiscard]] ignorado está em erros/. */
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

enum { EVT_START, EVT_RESUME };

static void start(void)  { puts("  start"); }
static void resume(void) { puts("  resume"); }

[[nodiscard("verifique o código de erro")]]
int flash_write(uint32_t addr, const void *data, size_t len);

int flash_write(uint32_t addr, const void *data, size_t len)
{
    (void)data;
    printf("flash_write(0x%08lx, %zu bytes)\n", (unsigned long)addr, len);
    return 0;
}

void handle(int event, [[maybe_unused]] void *ctx)
{
    switch (event) {
    case EVT_START:
        start();
        [[fallthrough]];
    case EVT_RESUME:
        resume();
        break;
    }
}

int main(void)
{
    const uint8_t dados[4] = { 0xde, 0xad, 0xbe, 0xef };

    if (flash_write(0x0800'0000, dados, sizeof dados) != 0) {   /* retorno verificado */
        return 1;
    }

    puts("handle(EVT_START):");
    handle(EVT_START, nullptr);
    puts("handle(EVT_RESUME):");
    handle(EVT_RESUME, nullptr);
    return 0;
}
