#ifndef __SWAP_H__
#define __SWAP_H__

#include "config.h"	// macros, bool, uint[XX]_t

// Check for BSD or Linux-style interface
#ifdef HAVE_ENDIAN_H
#	include <endian.h>	// be??toh
#elif defined(HAVE_SYS_ENDIAN_H)
#	include <sys/endian.h>	// see endian.h (but for BSD)
#elif defined(__APPLE__)
#	include <libkern/OSByteOrder.h>
#	define be16toh(x) OSSwapBigToHostInt16(x)
#	define le16toh(x) OSSwapLittleToHostInt16(x)
#	define htobe16(x) OSSwapHostToBigInt16(x)
#	define htole16(x) OSSwapHostToLittleInt16(x)
#	define be32toh(x) OSSwapBigToHostInt16(x)
#	define le32toh(x) OSSwapLittleToHostInt16(x)
#	define htobe32(x) OSSwapHostToBigInt32(x)
#	define htole32(x) OSSwapHostToLittleInt32(x)
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

static inline uint16_t htole16(uint16_t host_16bits)
{
#if defined(LITTLE_ENDIAN)
	return host_16bits;
#else
	return be16toh(host_16bits);
#endif /*LITTLE_ENDIAN*/
}

static inline uint16_t htobe16(uint16_t host_16bits)
{
#if defined(BIG_ENDIAN)
	return host_16bits;
#else
	return le16toh(host_16bits);
#endif
}

//!
static inline uint32_t be32toh(uint32_t big_endian_32bits)
{
#if defined(BIG_ENDIAN)
	return big_endian_32bits;
#elif defined(HAVE_BSWAP_32)
	return __bswap_32(big_endian_32bits);
#elif defined(HAVE_BYTESWAP_ULONG)
	return _byteswap_ulong(big_endian_32bits);
#else
	// TODO make our own builtins
	return 0;
#error "Your platform has no sensible builtins for endianness-swapping"
#endif /*BIG_ENDIAN*/
}

static inline uint32_t le32toh(uint32_t little_endian_32bits)
{
#if defined(LITTLE_ENDIAN)
	return little_endian_32bits;
#elif defined(HAVE_BSWAP_32)
	return __bswap_32(little_endian_32bits);
#elif defined(HAVE_BYTESWAP_ULONG)
	return _byteswap_ulong(little_endian_32bits);
#else
	// TODO make our own builtins
	return 0;
#error "Your platform has no sensible builtins for endianness-swapping"
#endif /*LITTLE_ENDIAN*/
}

static inline uint32_t htole32(uint32_t host_32bits)
{
#if defined(LITTLE_ENDIAN)
	return host_32bits;
#else
	return be32toh(host_32bits);
#endif /*LITTLE_ENDIAN*/
}

static inline uint32_t htobe32(uint32_t host_32bits)
{
#if defined(BIG_ENDIAN)
	return host_32bits;
#else
	return le32toh(host_32bits);
#endif
}

#endif /*HAVE_ENDIAN_H || HAVE_SYS_ENDIAN_H*/

#endif /*__SWAP_H__*/
