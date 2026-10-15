/* O ring buffer só usa load e store atômicos: compila e linka até no Cortex-M0, sem nenhuma
 * função __atomic_*, só com DMBs. */
#include "../host/ringbuf.h"

ringbuf_t rb;

bool poe(uint32_t v)   { return rb_poe(&rb, v); }
bool tira(uint32_t *v) { return rb_tira(&rb, v); }
