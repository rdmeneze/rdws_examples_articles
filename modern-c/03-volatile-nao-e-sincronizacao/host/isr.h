/* Uma "ISR" de mentira para rodar os exemplos no host: um sinal SIGALRM a cada 20 µs.
 * Usa sigaction porque, com -std=c11, o signal() da glibc desarma o tratador depois do
 * primeiro sinal. */
#ifndef ISR_H
#define ISR_H

#include <signal.h>
#include <sys/time.h>

static inline void isr_liga(void (*isr)(int))
{
    struct sigaction sa = { .sa_handler = isr };
    struct itimerval t = { .it_interval = { 0, 20 }, .it_value = { 0, 20 } };

    sigemptyset(&sa.sa_mask);
    sigaction(SIGALRM, &sa, NULL);
    setitimer(ITIMER_REAL, &t, NULL);
}

static inline void isr_desliga(void)
{
    struct itimerval z = { 0 };

    setitimer(ITIMER_REAL, &z, NULL);
}

/* O equivalente a desligar as interrupções: bloquear o sinal. */
static inline void irq_desliga(sigset_t *antes)
{
    sigset_t s;

    sigemptyset(&s);
    sigaddset(&s, SIGALRM);
    sigprocmask(SIG_BLOCK, &s, antes);
}

static inline void irq_restaura(const sigset_t *antes)
{
    sigprocmask(SIG_SETMASK, antes, NULL);
}

#endif
