#ifndef __PLATFORM_COMPILER_GCC_H__
#define __PLATFORM_COMPILER_GCC_H__

// Clobber previous definitions with extreme prejudice
#ifdef UNUSED
#	undef UNUSED
#endif
#ifdef likely
#	undef likely
#endif
#ifdef unlikely
#	undef unlikely
#endif
#ifdef NORETURN
#	undef NORETURN
#endif

#define UNUSED __attribute__((__unused__))
#define unlikely(x) (!!__builtin_expect((x), 0))
#define likely(x) (!!__builtin_expect((x), 1))

#if __STDC_VERSION__ >= 201112L
#	define NORETURN _Noreturn
#else
#	define NORETURN __attribute__((noreturn))
#endif

#endif /*__PLATFORM_COMPILER_GCC_H__*/
