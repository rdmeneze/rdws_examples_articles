/* C11: _Noreturn indica que a função nunca retorna, como um tratador de falhas. */
#include <stdio.h>
#include <stdlib.h>

_Noreturn void fault_handler(void);

void fault_handler(void)
{
    puts("fault_handler: registrando a falha e reiniciando");
    exit(EXIT_SUCCESS);   /* no firmware, seria um reset (NVIC_SystemReset no Cortex-M) */
}

int main(void)
{
    fault_handler();
    /* nada depois daqui é alcançado; o compilador sabe disso */
}
