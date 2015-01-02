#ifndef __SWAP_H__
#define __SWAP_H__

#include "config.h"	// macros, bool, uint[XX]_t

// Check for BSD or Linux-style interface
#if defined(HAVE_ENDIAN_H)
#	include <endian.h>	// be??toh
#elif defined(HAVE_SYS_ENDIAN_H)
#	include <sys/endian.h>	// see endian.h (but for BSD)
#elif defined(__APPLE__)
#	include <libkern/OSByteOrder.h>
#	define be16toh(x) OSSwapBigToHostInt16(x)
#	define le16toh(x) OSSwapLittleToHostInt16(x)
#	define htobe16(x) OSSwapHostToBigInt16(x)
#	define htole16(x) OSSwapHostToLittleInt16(x)
#	define be32toh(x) OSSwapBigToHostInt32(x)
#	define le32toh(x) OSSwapLittleToHostInt32(x)
#	define htobe32(x) OSSwapHostToBigInt32(x)
#	define htole32(x) OSSwapHostToLittleInt32(x)
#else

// Implement a BSD-like interface as a fallback
#if defined(HAVE_BSWAP_16)
#	define SWAP16(n) __bswap_16(n)
#	define SWAP32(n) __bswap_32(n)
#elif defined(HAVE_BYTESWAP_USHORT)
#	define SWAP16(n) _byteswap_ushort(n)
#	define SWAP32(n) _byteswap_ulong(n)
#else
#	define SWAP16(n) (((uint16_t)(n) >> 8) | ((uint16_t)(n) << 8))
#	define SWAP32(n) (((uint32_t)(n) >> 16) | ((uint32_t)(n) << 16))
#endif /*defined(HAVE_BSWAP_16) || defined(HAVE_BYTESWAP_USHORT)*/

static inline uint16_t be16toh(uint16_t big_endian_16bits)
{
#if defined(BIG_ENDIAN)
	return big_endian_16bits;
#else
	return SWAP16(big_endian_16bits);
#endif /*BIG_ENDIAN*/
}

static inline uint16_t le16toh(uint16_t little_endian_16bits)
{
#if defined(LITTLE_ENDIAN)
	return little_endian_16bits;
#else
	return SWAP16(little_endian_16bits);
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
#endif /*BIG_ENDIAN*/
}

static inline uint32_t be32toh(uint32_t big_endian_32bits)
{
#if defined(BIG_ENDIAN)
	return big_endian_32bits;
#else
	return SWAP32(big_endian_32bits);
#endif /*BIG_ENDIAN*/
}

static inline uint32_t le32toh(uint32_t little_endian_32bits)
{
#if defined(LITTLE_ENDIAN)
	return little_endian_32bits;
#else
	return SWAP32(little_endian_32bits);
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
#endif /*BIG_ENDIAN*/
}

#endif /*defined(HAVE_ENDIAN_H) || defined(HAVE_SYS_ENDIAN_H) || defined(__APPLE__)*/

#endif /*__SWAP_H__*/
