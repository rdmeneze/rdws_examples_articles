static int erro;
int envia(int n)
{
    int erro = 0;                 /* esconde a variável global */
    if (n < 0) erro = -1;
    return erro;
}
int ultimo_erro(void) { return erro; }
