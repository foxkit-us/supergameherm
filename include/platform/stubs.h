#ifndef __PLATFORM_STUBS_H__
#define __PLATFORM_STUBS_H__

#if defined(HAVE_STDBOOL_H)
#	include <stdbool.h>
#elif defined(HAVE_COMPILER_MSVC)
	// this is fine, msvc has intrinsic bool without header
#else
#	include "platform/stdbool_stub.h" // Fake it
#endif

#if defined(HAVE_STDINT_H)
#	include <stdint.h>
#elif defined(HAVE_COMPILER_MSVC)
	// Ignore, we include in the compiler platform already
#else
#	error "Your compiler doesn't appear to have C99 support."
#endif

#endif /*__PLATFORM_STUBS_H__*/
