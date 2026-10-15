/* tipo: linguagem; verifica: compila; recurso: nullptr e nullptr_t */
#include <stddef.h>
int main(void) { int *p = nullptr; nullptr_t n = nullptr; return p == n ? 0 : 1; }
