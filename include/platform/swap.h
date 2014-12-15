#ifndef __SWAP_H__
#define __SWAP_H__

#include "config.h"	// macros, bool, uint[XX]_t

// Check for BSD or Linux-style interface
#ifdef HAVE_ENDIAN_H
#	include <endian.h>	// be??toh
#elif HAVE_SYS_ENDIAN_H
#	include <sys/endian.h>	// see endian.h (but for BSD)
#elif defined(__APPLE__)
#   include <libkern/OSByteOrder.h>
#   define be16toh(x) OSSwapBigToHostInt16(x)
#   define le16toh(x) OSSwapLittleToHostInt16(x)
#else

// Implement a BSD-like interface as a fallback

static inline uint16_t be16toh(uint16_t big_endian_16bits)
{
#if defined(BIG_ENDIAN)
	return big_endian_16bits;
#elif defined(HAVE_BSWAP_16)
	return __bswap_16(big_endian_16bits);
#elif defined(HAVE_BYTESWAP_USHORT)
	return _byteswap_ushort(big_endian_16bits);
#else
	// TODO make our own builtins
	return 0;
#error "Your platform has no sensible builtins for endianness-swapping"
#endif /*BIG_ENDIAN*/
}

static inline uint16_t le16toh(uint16_t little_endian_16bits)
{
#if defined(LITTLE_ENDIAN)
	return little_endian_16bits;
#elif defined(HAVE_BSWAP_16)
	return __bswap_16(little_endian_16bits);
#elif defined(HAVE_BYTESWAP_USHORT)
	return _byteswap_ushort(little_endian_16bits);
#else
	// TODO make our own builtins
	return 0;
#error "Your platform has no sensible builtins for endianness-swapping"
#endif /*LITTLE_ENDIAN*/
}

#endif /*HAVE_ENDIAN_H || HAVE_SYS_ENDIAN_H*/

#endif /*__SWAP_H__*/
