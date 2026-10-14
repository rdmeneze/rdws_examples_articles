/* Deve FALHAR: a tabela tem menos entradas que o enum. Com -DTABELA_COMPLETA, compila. */
#include <stdint.h>
#include <stdio.h>

enum sensor_id { SENSOR_TEMP, SENSOR_HUMIDITY, SENSOR_PRESSURE, SENSOR_COUNT };

typedef struct {
    uint8_t i2c_addr;
    uint16_t period_ms;
} sensor_cfg_t;

static const sensor_cfg_t sensor_cfg[] = {
    [SENSOR_TEMP]     = { .i2c_addr = 0x44, .period_ms = 1000 },
    [SENSOR_HUMIDITY] = { .i2c_addr = 0x44, .period_ms = 1000 },
#ifdef TABELA_COMPLETA
    [SENSOR_PRESSURE] = { .i2c_addr = 0x77, .period_ms = 5000 },
#endif
};

static_assert(sizeof sensor_cfg / sizeof sensor_cfg[0] == SENSOR_COUNT,
              "sensor_cfg precisa ter uma entrada por sensor");

int main(void)
{
    printf("%zu sensores configurados\n", sizeof sensor_cfg / sizeof sensor_cfg[0]);
    return 0;
}
