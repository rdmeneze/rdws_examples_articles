/* tipo: linguagem; verifica: compila; recurso: #embed */
static const unsigned char dados[] = {
#embed "dados.bin"
};
int main(void) { return sizeof dados == 4 ? 0 : 1; }
