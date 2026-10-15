/* tipo: cabeçalho do compilador; verifica: compila; recurso: unreachable() em stddef.h */
#include <stddef.h>
static int f(int x) { if (x == 0) { return 1; } unreachable(); }
int main(void) { return f(0) == 1 ? 0 : 1; }
