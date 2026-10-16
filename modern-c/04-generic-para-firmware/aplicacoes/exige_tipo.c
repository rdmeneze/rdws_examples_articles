/* Verificação de tipo em tempo de compilação: _Generic dentro de um static_assert.
 * Com -DERRADO, o contador de 16 bits quebra o build. */
#include <stdint.h>

#define EXIGE_TIPO(x, T) \
    static_assert(_Generic((x), T: 1, default: 0), "tipo errado: " #x " precisa ser " #T)

uint32_t timestamp;
EXIGE_TIPO(timestamp, uint32_t);

#ifdef ERRADO
uint16_t contador;
EXIGE_TIPO(contador, uint32_t);
#endif
