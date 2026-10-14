/* C23: stdbit.h e unreachable(). FW_UNREACHABLE registra onde o problema aconteceu em
 * desenvolvimento e vira a dica de otimização com NDEBUG. */
#include <stdbit.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

enum sensor_state : uint8_t {
    SENSOR_IDLE,
    SENSOR_MEASURING,
    SENSOR_ERROR,
};

[[noreturn]] void fw_panic(const char *file, int line);

void fw_panic(const char *file, int line)
{
    fprintf(stderr, "fw_panic: %s:%d\n", file, line);
    abort();   /* no firmware: guardar em RAM preservada e reiniciar */
}

#ifdef NDEBUG
#define FW_UNREACHABLE() unreachable()
#else
#define FW_UNREACHABLE() fw_panic(__FILE__, __LINE__)
#endif

/* Como no artigo: a promessa pura. */
static const char *state_name(enum sensor_state state)
{
    switch (state) {
    case SENSOR_IDLE:      return "IDLE";
    case SENSOR_MEASURING: return "MEASURING";
    case SENSOR_ERROR:     return "ERROR";
    default:
        unreachable();
    }
}

/* Com a macro do projeto: fw_panic em desenvolvimento, unreachable() com NDEBUG. */
static const char *state_name_fw(enum sensor_state state)
{
    switch (state) {
    case SENSOR_IDLE:      return "IDLE";
    case SENSOR_MEASURING: return "MEASURING";
    case SENSOR_ERROR:     return "ERROR";
    default:
        FW_UNREACHABLE();
    }
}

int main(void)
{
    uint32_t mask = 0b1011'0000;

    printf("mask = 0x%02lx\n", (unsigned long)mask);
    printf("stdc_count_ones     = %u\n", stdc_count_ones(mask));
    printf("stdc_trailing_zeros = %u\n", stdc_trailing_zeros(mask));
    printf("stdc_leading_zeros  = %u\n", stdc_leading_zeros(mask));
    printf("stdc_has_single_bit(64) = %d\n", (int)stdc_has_single_bit(64u));
    printf("stdc_bit_width(mask)    = %u\n", stdc_bit_width(mask));

    printf("state_name(SENSOR_MEASURING)    = %s\n", state_name(SENSOR_MEASURING));
    printf("state_name_fw(SENSOR_ERROR)     = %s\n", state_name_fw(SENSOR_ERROR));
    return 0;
}
