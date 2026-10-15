/* tipo: linguagem; verifica: compila; recurso: typeof e typeof_unqual */
int main(void)
{
    const int a = 1;
    typeof(a) b = 2;
    typeof_unqual(a) c = 3;
    c = b;
    return c == 2 ? 0 : 1;
}
