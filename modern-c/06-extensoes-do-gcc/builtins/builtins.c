/* Builtins do GCC que todo firmware acaba usando. */
#include <stdint.h>
#include <stdio.h>

#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define IS_CONST(x)      __builtin_constant_p(x)
#define SAME_TYPE(a, b)  __builtin_types_compatible_p(__typeof__(a), __typeof__(b))

int main(int argc, char **argv)
{
    (void)argv;
    uint32_t reg = 0x12345678;
    uint32_t mask = 0xb0;
    uint16_t total;

    if (unlikely(argc > 10)) {
        return 1;
    }

    printf("__builtin_bswap32(0x%08lx) = 0x%08lx\n",
           (unsigned long)reg, (unsigned long)__builtin_bswap32(reg));
    printf("mask = 0x%02lx: popcount = %d, ctz = %d, clz = %d\n", (unsigned long)mask,
           __builtin_popcount(mask), __builtin_ctz(mask), __builtin_clz(mask));

    if (__builtin_add_overflow((uint16_t)65000, (uint16_t)1000, &total)) {
        printf("__builtin_add_overflow: overflow (total truncado = %u)\n", total);
    }

    printf("__builtin_constant_p(42) = %d, __builtin_constant_p(argc) = %d\n",
           IS_CONST(42), IS_CONST(argc));
    printf("mesmo tipo (reg, mask) = %d, (reg, total) = %d\n",
           SAME_TYPE(reg, mask), SAME_TYPE(reg, total));
    return 0;
}
