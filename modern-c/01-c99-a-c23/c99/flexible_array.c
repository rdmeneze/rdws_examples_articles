/* C99: flexible array member para pacotes de tamanho variável. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    uint8_t id;
    uint8_t len;
    uint8_t payload[];
} packet_t;

int main(void)
{
    const char dados[] = "21.5C";
    size_t n = sizeof dados - 1;

    packet_t *p = malloc(sizeof *p + n);
    if (p == NULL) {
        return 1;
    }

    p->id = 7;
    p->len = (uint8_t)n;
    memcpy(p->payload, dados, n);

    printf("sizeof(packet_t) = %zu (só o cabeçalho)\n", sizeof(packet_t));
    printf("pacote: id=%u len=%u payload=%.*s\n", p->id, p->len, (int)p->len, (const char *)p->payload);

    free(p);
    return 0;
}
