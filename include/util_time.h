#ifndef __UTIL_TIME_H__
#define __UTIL_TIME_H__

#include "config.h"		// macros, bool, uint[XX]_t

// Include the appropriate time functions
#ifdef HAVE_CLOCK_GETTIME
#	include "platform/time_posix.h"
#elif defined(HAVE_MACH_CLOCK_H)
#	include "platform/time_mach.h"
#else
#	include "platform/time_null.h"
#endif

#ifdef HAVE_NANOSLEEP
#	include "platform/sleep_posix.h"
#else
#	include "platform/sleep_null.h"
#endif

#endif /*__UTIL_TIME_H__*/
