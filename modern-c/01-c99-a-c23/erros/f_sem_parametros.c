/* Compila com -std=c17 e FALHA com C23: "void f();" agora significa "sem parâmetros",
 * como "void f(void)", e não mais "parâmetros não especificados". */
#include <stdio.h>

void report();

int main(void)
{
    report(3);
    return 0;
}

void report(int n)
{
    printf("report(%d)\n", n);
}
