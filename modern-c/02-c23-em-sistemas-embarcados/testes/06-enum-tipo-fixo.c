/* tipo: linguagem; verifica: compila; recurso: enum com tipo subjacente */
enum estado : unsigned char { OCIOSO, MEDINDO };
static_assert(sizeof(enum estado) == 1);
int main(void) { return MEDINDO == 1 ? 0 : 1; }
