/* atomic_fetch_add: no M4 vira LDREX/STREX; no M0 vira uma chamada a __atomic_fetch_add_4,
 * que o toolchain do Zephyr SDK não fornece: o link falha. */
#include <stdatomic.h>

atomic_uint contador;

int main(void)
{
    atomic_fetch_add(&contador, 1);
    return (int)atomic_load(&contador);
}
