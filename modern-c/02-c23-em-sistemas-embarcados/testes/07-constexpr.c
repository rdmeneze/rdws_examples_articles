/* tipo: linguagem; verifica: compila; recurso: constexpr */
constexpr unsigned HZ = 1000;
constexpr unsigned RELOAD = 48000000 / HZ - 1;
static_assert(RELOAD == 47999);
int main(void) { return 0; }
