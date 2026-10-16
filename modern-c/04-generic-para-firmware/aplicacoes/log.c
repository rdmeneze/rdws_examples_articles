/* Log com o formato do printf escolhido pelo tipo. O -Wformat continua conferindo a string
 * escolhida: o teste troca um formato por um errado e confere o aviso. */
#include <stdint.h>
#include <stdio.h>
/* (x) + 0 aplica as promoções inteiras: short e char viram int. */
#define FMT(x) _Generic((x) + 0, \
    int: "%d", unsigned int: "%u", long: "%ld", unsigned long: "%lu", \
    long long: "%lld", unsigned long long: "%llu", double: "%f", char *: "%s")
#define LOG_VAL(nome, x) do {  \
    printf(nome " = ");        \
    printf(FMT(x), (x));       \
    printf("\n");              \
} while (0)
int main(void)
{
    uint32_t a = 4000000000u; int16_t b = -5; double c = 1.5; char *s = "ok"; uint64_t d = 1ull << 40;
    LOG_VAL("a", a); LOG_VAL("b", b); LOG_VAL("c", c); LOG_VAL("s", s); LOG_VAL("d", d);
    return 0;
}
