/* Só pronto é volatile. O compilador pode eliminar a primeira escrita em dado e fazer a
 * segunda depois de pronto = 1: quem vê pronto == 1 lê um dado antigo. */
int dado;
volatile int pronto;

void publica(int v)
{
    dado = v;
    pronto = 1;
    dado = v + 1;
}
