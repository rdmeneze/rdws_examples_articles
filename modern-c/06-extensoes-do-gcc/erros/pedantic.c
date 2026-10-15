/* Com -pedantic-errors: __typeof__ passa (grafia reservada), mas a statement expression
 * falha, porque é uma extensão de sintaxe. */
int main(void)
{
    __typeof__(1) a = 1;
    int b = ({ int t = a; t + 1; });
    return b - 2;
}
