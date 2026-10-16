[[deprecated("use sensor_read_v2(), que retorna o código de erro")]]
int sensor_read(void);

int le(void)
{
    return sensor_read();
}
