/* static_assert para premissas de configuração. Compile com -DRB_TAMANHO=100 para ver o erro. */
#include <limits.h>
#include <stdatomic.h>
#include <stdint.h>

#ifndef RB_TAMANHO
#define RB_TAMANHO 64u
#endif

/* O ring buffer do artigo 03 usa cab % RB_TAMANHO: com potência de 2, vira um AND. */
static_assert((RB_TAMANHO & (RB_TAMANHO - 1)) == 0, "RB_TAMANHO precisa ser potência de 2");

/* Premissas da plataforma. */
static_assert(CHAR_BIT == 8, "este código supõe bytes de 8 bits");
static_assert(ATOMIC_INT_LOCK_FREE == 2, "atomic_int precisa ser lock-free para uso em ISR");

/* Um pacote precisa caber no payload de um quadro. */
#define MTU_PAYLOAD 64u
typedef struct {
    uint8_t  tipo;
    uint8_t  sequencia;
    uint16_t medidas[30];
} pacote_t;
static_assert(sizeof(pacote_t) <= MTU_PAYLOAD, "pacote_t não cabe no payload");

uint8_t buffer[RB_TAMANHO];
