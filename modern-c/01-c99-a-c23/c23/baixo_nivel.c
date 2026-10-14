/* C23: literais binários, separadores numéricos e enums com tipo subjacente fixo. */
#include <stdint.h>
#include <stdio.h>

#define UART_CR1_TE   0b0000'1000
#define CPU_FREQ_HZ   480'000'000

enum sensor_state : uint8_t {
    SENSOR_IDLE,
    SENSOR_MEASURING,
    SENSOR_ERROR,
};

static_assert(sizeof(enum sensor_state) == 1);

int main(void)
{
    printf("UART_CR1_TE = 0x%02x\n", UART_CR1_TE);
    printf("CPU_FREQ_HZ = %lu\n", (unsigned long)CPU_FREQ_HZ);
    printf("sizeof(enum sensor_state) = %zu\n", sizeof(enum sensor_state));
    return 0;
}
