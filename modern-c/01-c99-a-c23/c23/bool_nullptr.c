/* C23: bool vira palavra-chave (sem stdbool.h) e nullptr tem tipo próprio, nullptr_t. */
#include <stddef.h>
#include <stdio.h>

#define TIPO(x) _Generic((x),        \
    int:       "int",                \
    long:      "long",               \
    void *:    "void *",             \
    nullptr_t: "nullptr_t",          \
    default:   "outro")

static void device_init(void *cfg)
{
    printf("device_init(%s)\n", cfg == nullptr ? "nullptr" : "cfg");
}

int main(void)
{
    bool enabled = true;
    device_init(nullptr);

    printf("enabled = %d\n", enabled);
    /* O tipo de NULL depende de como a implementação o define. */
    printf("NULL nesta implementação é do tipo: %s\n", TIPO(NULL));
    printf("nullptr é do tipo: %s\n", TIPO(nullptr));
    return 0;
}
