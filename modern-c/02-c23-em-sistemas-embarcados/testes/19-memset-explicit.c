/* tipo: biblioteca; verifica: linka; recurso: memset_explicit */
#include <string.h>
int main(void) {
    char chave[16] = "segredo"; memset_explicit(chave, 0, sizeof chave);
    return chave[0];
}
