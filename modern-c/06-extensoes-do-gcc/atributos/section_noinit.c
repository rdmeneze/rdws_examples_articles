/* section: coloca uma variável em uma seção escolhida. Em firmware, uma seção .noinit
 * (que o linker script não zera no boot) guarda dados que sobrevivem a um reset. */
#include <stdint.h>
#include <stdio.h>

__attribute__((section(".noinit"))) static uint32_t boot_count;

int main(void)
{
    boot_count++;   /* no host, a seção é zerada como qualquer outra; no firmware, não */
    printf("&boot_count = %p, boot_count = %lu\n", (void *)&boot_count, (unsigned long)boot_count);
    return 0;
}
