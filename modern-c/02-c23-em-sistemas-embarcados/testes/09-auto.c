/* tipo: linguagem; verifica: compila; recurso: auto (inferência de tipo) */
int main(void)
{
    auto x = 1000u;
    static_assert(_Generic(x, unsigned int: 1, default: 0));
    return 0;
}
