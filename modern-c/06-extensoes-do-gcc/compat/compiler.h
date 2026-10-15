/* compiler.h: isola as extensões do compilador em um lugar só, no padrão do
 * include/linux/compiler_attributes.h e do include/zephyr/toolchain/gcc.h.
 *
 * Regra: o que muda o significado do programa (PACKED, WEAK, SECTION) dá erro quando o
 * compilador não suporta; o que é só diagnóstico ou dica (PRINTF_FMT, likely) vira nada. */
#ifndef COMPILER_H
#define COMPILER_H

#include <stdint.h>

#if defined(__has_attribute) && !defined(COMPAT_SIMULA_OUTRO_COMPILADOR)
#define TEM_ATRIBUTO(x) __has_attribute(x)
#else
#define TEM_ATRIBUTO(x) 0
#endif

#if defined(__has_builtin) && !defined(COMPAT_SIMULA_OUTRO_COMPILADOR)
#define TEM_BUILTIN(x) __has_builtin(x)
#else
#define TEM_BUILTIN(x) 0
#endif

/* Mudam o significado: sem suporte, não há substituto seguro. */
#if TEM_ATRIBUTO(packed)
#define PACKED __attribute__((__packed__))
#else
#define PACKED _Pragma("GCC error \"PACKED: defina para este compilador\"")
#endif

#if TEM_ATRIBUTO(weak)
#define WEAK __attribute__((__weak__))
#else
#define WEAK _Pragma("GCC error \"WEAK: defina para este compilador\"")
#endif

#if TEM_ATRIBUTO(section)
#define SECTION(s) __attribute__((__section__(s)))
#else
#define SECTION(s) _Pragma("GCC error \"SECTION: defina para este compilador\"")
#endif

/* Só diagnóstico ou dica: sem suporte, some. */
#if TEM_ATRIBUTO(format)
#define PRINTF_FMT(f, a) __attribute__((__format__(printf, f, a)))
#else
#define PRINTF_FMT(f, a)
#endif

#if TEM_BUILTIN(__builtin_expect)
#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define likely(x)   (x)
#define unlikely(x) (x)
#endif

/* Builtin com substituto portável. */
#if TEM_BUILTIN(__builtin_bswap32)
#define BSWAP32(x) __builtin_bswap32(x)
#else
static inline uint32_t compat_bswap32(uint32_t x)
{
    return (x >> 24) | ((x >> 8) & 0xff00u) | ((x << 8) & 0xff0000u) | (x << 24);
}
#define BSWAP32(x) compat_bswap32(x)
#endif

#endif /* COMPILER_H */
