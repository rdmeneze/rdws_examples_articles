/* Função aninhada que captura uma variável local: exige um trampolim na pilha, e a pilha
 * passa a precisar ser executável. Com -Werror=trampolines, deve FALHAR. */
static void para_cada(void (*cb)(int), int n)
{
    for (int i = 0; i < n; i++) {
        cb(i);
    }
}

int main(void)
{
    int soma = 0;
    void acumula(int x) { soma += x; }

    para_cada(acumula, 4);
    return soma == 6 ? 0 : 1;
}
