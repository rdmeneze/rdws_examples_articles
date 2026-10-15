/* Roda as funções de barreira.c com um "ISR" de mentira: um sinal que liga flag depois
 * de 200 ms. Escrever em um int comum dentro do tratador de sinal é justamente o erro que
 * o exemplo mostra (o certo seria volatile sig_atomic_t ou um atomic).
 *
 * Uso: espera sem|com. O script de teste roda com timeout, porque "sem" pode travar. */
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

extern int flag;
void espera_sem_barreira(void);
void espera_com_barreira(void);

static void isr(int sig)
{
    (void)sig;
    flag = 1;
}

int main(int argc, char **argv)
{
    struct itimerval t = { .it_value = { .tv_sec = 0, .tv_usec = 200000 } };

    signal(SIGALRM, isr);
    setitimer(ITIMER_REAL, &t, NULL);

    if (argc > 1 && strcmp(argv[1], "com") == 0) {
        espera_com_barreira();
    } else {
        espera_sem_barreira();
    }
    printf("retornou com flag = %d\n", flag);
    return 0;
}
