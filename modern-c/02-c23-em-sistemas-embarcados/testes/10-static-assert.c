/* tipo: linguagem; verifica: compila; recurso: static_assert sem mensagem, sem assert.h */
static_assert(sizeof(int) >= 2);
int main(void) { return 0; }
