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
#ifdef alignment
#	undef alignment
#endif

#define UNUSED __attribute__((__unused__))
#define unlikely(x) (!!__builtin_expect((x), 0))
#define likely(x) (!!__builtin_expect((x), 1))
#define alignment(x) __attribute__((aligned(x)))

#endif /*__PLATFORM_COMPILER_GCC_H__*/
