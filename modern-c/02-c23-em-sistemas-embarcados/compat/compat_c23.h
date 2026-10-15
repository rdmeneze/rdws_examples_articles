/* compat_c23.h: usa o recurso do C23 quando existe e um substituto quando não existe.
 * Testado de -std=c11 a -std=c23 (ver scripts/teste_compat.sh). */
#ifndef COMPAT_C23_H
#define COMPAT_C23_H

#include <stddef.h>
#include <string.h>

/* 201710L é o C17. GCC 13 e 14 definem 202000L com -std=c2x/c23, então não use
 * __STDC_VERSION__ >= 202311L para detectar o C23. */
#if __STDC_VERSION__ > 201710L
#define COMPAT_C23 1
#else
#define COMPAT_C23 0
#endif

#if !COMPAT_C23
#include <stdbool.h>
#include <assert.h>              /* static_assert como macro, no C11/C17 */
#define nullptr ((void *)0)
#endif

/* Recursos de biblioteca: teste um por um, não pela versão do padrão. */
#if defined(__has_include)
#if __has_include(<stdckdint.h>)
#include <stdckdint.h>
#define COMPAT_TEM_CKDINT 1
#endif
#endif

#ifndef COMPAT_TEM_CKDINT
#define ckd_add(r, a, b) __builtin_add_overflow((a), (b), (r))
#define ckd_sub(r, a, b) __builtin_sub_overflow((a), (b), (r))
#define ckd_mul(r, a, b) __builtin_mul_overflow((a), (b), (r))
#endif

/* memset_explicit não tem macro de teste: o build define HAVE_MEMSET_EXPLICIT. A barreira
 * impede o compilador de remover o memset (extensão do GCC/Clang). */
#ifndef HAVE_MEMSET_EXPLICIT
static inline void *compat_memset_explicit(void *p, int c, size_t n)
{
    memset(p, c, n);
    __asm__ volatile("" : : "r"(p) : "memory");
    return p;
}
#define memset_explicit compat_memset_explicit
#endif

#endif /* COMPAT_C23_H */
