#pragma once

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

#define unused __pragma(warning(disable:4100))
#define unlikely(x) (x)
#define likely(x) (x)
#if (_MSC_VER >= 1400)
#	define restrict __restrict
#else
#	define restrict
#endif

typedef signed __int8 int8_t;
typedef signed __int16 int16_t;
typedef signed __int32 int32_t;
typedef signed __int64 int64_t;

typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
