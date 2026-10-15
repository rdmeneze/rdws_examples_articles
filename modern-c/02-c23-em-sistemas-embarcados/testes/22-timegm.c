/* tipo: biblioteca; verifica: linka; recurso: timegm (novo no padrão C23) */
#include <time.h>
int main(void) { struct tm t = { .tm_year = 126, .tm_mon = 9, .tm_mday = 14 }; return timegm(&t) > 0 ? 0 : 1; }
