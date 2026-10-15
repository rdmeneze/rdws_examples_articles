/* tipo: linguagem; verifica: compila; recurso: __VA_OPT__ */
#define CONTA(...) (0 __VA_OPT__(+ 1))
int main(void) { return CONTA() == 0 && CONTA(x) == 1 ? 0 : 1; }
