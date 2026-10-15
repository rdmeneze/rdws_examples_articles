/* tipo: biblioteca; verifica: linka; recurso: stdbit.h (stdc_count_ones) */
#include <stdbit.h>
int main(void) { volatile unsigned v = 0xb0u; return stdc_count_ones(v) == 3 ? 0 : 1; }
