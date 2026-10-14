/* C11: structs e unions anônimas, acessadas diretamente pelo objeto externo. */
#include <stdint.h>
#include <stdio.h>

typedef struct {
    uint8_t type;
    union {
        struct {
            int16_t  temperature;
            uint16_t humidity;
        };
        uint32_t raw;
    };
} reading_t;

int main(void)
{
    reading_t r;
    r.temperature = 215;   /* sem r.u.s.temperature */
    r.humidity = 615;

    printf("temperature=%d humidity=%u raw=0x%08lx\n",
           r.temperature, r.humidity, (unsigned long)r.raw);
    return 0;
}
