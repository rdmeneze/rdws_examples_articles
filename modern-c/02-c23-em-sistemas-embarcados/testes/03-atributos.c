/* tipo: linguagem; verifica: compila; recurso: [[nodiscard]], [[maybe_unused]], [[fallthrough]], [[deprecated]], [[noreturn]] */
#include <stdlib.h>
[[nodiscard("verifique")]] static int f(void) { return 0; }
[[deprecated("use a nova")]] int velha(void);
[[noreturn]] static void para(void) { exit(0); }
int main(void)
{
    [[maybe_unused]] int nao_usada = 0;
    int r = f();
    switch (r) {
    case 0:
        r = 1;
        [[fallthrough]];
    case 1:
        break;
    }
    if (r == 2) {
        para();
    }
    return 0;
}
