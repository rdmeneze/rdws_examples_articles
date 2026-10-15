/* tipo: cabeçalho do compilador; verifica: linka; recurso: stdckdint.h (ckd_add) */
#include <stdckdint.h>
int main(void) { unsigned char r; return ckd_add(&r, 200, 100) ? 0 : 1; }
