/* tipo: biblioteca; verifica: linka; recurso: memccpy (novo no padrão C23) */
#include <string.h>
int main(void) { char d[8]; return memccpy(d, "ab;cd", ';', sizeof d) ? 0 : 1; }
