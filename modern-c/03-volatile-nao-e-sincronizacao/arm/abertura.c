/* O exemplo da abertura do artigo: parece correto e funciona na bancada, mas nada impede o
 * compilador de reordenar as escritas em amostras[] em relação a pronto (que é o único
 * volatile), e a próxima conversão pode sobrescrever o buffer enquanto processa() ainda lê. */
#include <stdbool.h>
#include <stdint.h>

uint16_t le_adc(void);
void processa(const uint16_t *dados, unsigned n);

static uint16_t amostras[64];
static volatile bool pronto;

void adc_isr(void)
{
    for (unsigned i = 0; i < 64; i++) {
        amostras[i] = le_adc();
    }
    pronto = true;
}

void tarefa(void)
{
    while (!pronto) {
    }
    pronto = false;
    processa(amostras, 64);
}
