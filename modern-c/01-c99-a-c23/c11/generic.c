/* C11: _Generic escolhe a função pelo tipo do argumento, em tempo de compilação. */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define abs_val(x) _Generic((x), \
    int:    abs,                 \
    long:   labs,                \
    float:  fabsf,               \
    double: fabs)(x)

int main(void)
{
    printf("abs_val(-3)    = %d\n", abs_val(-3));
    printf("abs_val(-4L)   = %ld\n", abs_val(-4L));
    printf("abs_val(-2.5f) = %.1f\n", (double)abs_val(-2.5f));
    printf("abs_val(-1.25) = %.2f\n", abs_val(-1.25));
    return 0;
}
