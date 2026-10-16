/* Qual tipo o _Generic enxerga? Literais, promoções, const e os typedefs de tamanho fixo. */
#include <stdint.h>
#include <stdio.h>

#define NOME(x) _Generic((x),                                          \
    char: "char", signed char: "signed char", unsigned char: "unsigned char", \
    short: "short", unsigned short: "unsigned short",                   \
    int: "int", unsigned int: "unsigned int",                           \
    long: "long", unsigned long: "unsigned long",                       \
    long long: "long long", unsigned long long: "unsigned long long",   \
    float: "float", double: "double", _Bool: "_Bool", default: "outro")

int main(void)
{
    const int c = 1;
    char ch = 'x';
    uint8_t u8 = 0;
    int32_t i32 = 0;
    uint32_t u32 = 0;

    printf("'a'      -> %s\n", NOME('a'));
    printf("ch       -> %s\n", NOME(ch));
    printf("ch + 1   -> %s\n", NOME(ch + 1));
    printf("const int-> %s\n", NOME(c));
    printf("uint8_t  -> %s\n", NOME(u8));
    printf("int32_t  -> %s\n", NOME(i32));
    printf("uint32_t -> %s\n", NOME(u32));
    printf("1 == 1   -> %s\n", NOME(1 == 1));
    return 0;
}
