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
#ifdef alignment
#	undef alignment
#endif

#define unused __pragma(warning(disable:4100))
#define unlikely(x) (x)
#define likely(x) (x)
#define alignment(x) __declspec(align(x))

#if (_MSC_VER >= 1400)
#	define restrict __restrict
#else
#	define restrict
#endif

#ifndef HAVE_STDINT_H
#	include "platform/os/stdint_msvc.h"
#endif
