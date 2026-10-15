/* tipo: linguagem; verifica: compila; recurso: inicializador vazio = {} */
struct cabecalho { unsigned char id; unsigned len; };
int main(void) { struct cabecalho h = {}; return h.id + (int)h.len; }
