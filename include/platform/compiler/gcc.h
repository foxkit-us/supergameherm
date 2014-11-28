#ifndef __PLATFORM_COMPILER_GCC_H__
#define __PLATFORM_COMPILER_GCC_H__

// Clobber previous definitions with extreme prejudice
#ifdef unused
#	undef unused
#endif
#ifdef likely
#	undef likely
#endif
#ifdef unlikely
#	undef unlikely
#endif

#define unused __attribute__((__unused__))
#define unlikely(x) (!!__builtin_expect((x), 0))
#define likely(x) (!!__builtin_expect((x), 1))

#endif /*__PLATFORM_COMPILER_GCC_H__*/
