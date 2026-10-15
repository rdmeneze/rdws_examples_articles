/* tipo: biblioteca; verifica: linka; recurso: strdup/strndup (novos no padrão C23) */
#include <stdlib.h>
#include <string.h>
int main(void) { char *a = strdup("abc"); char *b = strndup("abcdef", 2); int r = (a && b) ? 0 : 1; free(a); free(b); return r; }
