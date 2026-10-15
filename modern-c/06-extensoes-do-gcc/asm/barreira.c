/* Barreira de compilador: __asm__ volatile("" ::: "memory") não gera nenhuma instrução,
 * mas obriga o compilador a reler a memória depois dela. flag não é volatile de propósito. */
int flag;

#define barrier() __asm__ volatile("" ::: "memory")

void espera_sem_barreira(void)
{
    while (!flag) {
    }
}

void espera_com_barreira(void)
{
    while (!flag) {
        barrier();
    }
}
