/* cleanup: chama uma função quando a variável sai de escopo, em qualquer caminho de
 * saída. Aqui, garante que o "mutex" é liberado mesmo com return antecipado. */
#include <stdbool.h>
#include <stdio.h>

typedef struct { bool travado; } mutex_t;

static mutex_t bus_lock;

static void mutex_lock(mutex_t *m)   { m->travado = true; }
static void mutex_unlock(mutex_t *m) { m->travado = false; puts("  mutex liberado"); }

static void unlock_cleanup(mutex_t **m)
{
    mutex_unlock(*m);
}

#define SCOPED_LOCK(m) \
    mutex_t *scoped_lock_ __attribute__((cleanup(unlock_cleanup))) = (mutex_lock(m), (m))

static int le_sensor(bool falhar)
{
    SCOPED_LOCK(&bus_lock);

    if (falhar) {
        return -1;          /* o mutex é liberado aqui também */
    }
    return 42;
}

int main(void)
{
    printf("le_sensor(false) = %d\n", le_sensor(false));
    printf("le_sensor(true)  = %d\n", le_sensor(true));
    printf("travado no fim: %s\n", bus_lock.travado ? "sim" : "não");
    return 0;
}
