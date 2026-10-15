/* tipo: linguagem; verifica: compila; recurso: #elifdef / #elifndef */
#define B
#ifdef A
#error "A não devia estar definida"
#elifdef B
int main(void) { return 0; }
#elifndef C
#error "não devia chegar aqui"
#endif
