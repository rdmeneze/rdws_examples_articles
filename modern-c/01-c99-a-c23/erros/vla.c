/* Deve gerar AVISO com -Wvla: tamanho decidido em tempo de execução, na pilha.
 * O teste compila com -Werror=vla para transformar o aviso em falha. */
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

void process(size_t n)
{
    uint8_t buffer[n];   /* tamanho decidido em tempo de execução, na pilha */
    memset(buffer, 0, n);
    printf("buffer de %zu bytes na pilha\n", sizeof buffer);
}

int main(void)
{
    process(64);
    return 0;
}
