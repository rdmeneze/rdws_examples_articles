/* Compila com -std=c17 e FALHA com -std=gnu17 no host Linux: no modo GNU, o GCC define a
 * macro "linux" (com valor 1), um nome fora do espaço reservado. */
int linux = 1;

int main(void)
{
    return linux - 1;
}
