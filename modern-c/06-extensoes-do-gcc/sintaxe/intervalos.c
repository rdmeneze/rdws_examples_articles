/* Intervalos em case e em inicializadores designados. */
#include <stdio.h>

typedef void (*isr_t)(void);

static void default_handler(void) { }
static void uart_isr(void) { }

/* Todas as 16 posições apontam para default_handler, exceto a 5. Sobrescrever uma
 * posição do intervalo é intencional, mas -Wextra avisa (-Woverride-init); o aviso é
 * desligado só aqui. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverride-init"
static const isr_t vetores[16] = {
    [0 ... 15] = default_handler,
    [5] = uart_isr,
};
#pragma GCC diagnostic pop

static const char *classe(int codigo)
{
    switch (codigo) {
    case 200 ... 299: return "sucesso";
    case 400 ... 499: return "erro do cliente";
    case 500 ... 599: return "erro do servidor";
    default:          return "outro";
    }
}

int main(void)
{
    int padrao = 0;
    for (int i = 0; i < 16; i++) {
        padrao += vetores[i] == default_handler;
    }
    printf("vetores: %d com default_handler, [5] = %s\n",
           padrao, vetores[5] == uart_isr ? "uart_isr" : "?");
    printf("classe(204) = %s, classe(404) = %s\n", classe(204), classe(404));
    return 0;
}
