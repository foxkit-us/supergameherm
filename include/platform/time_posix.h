#ifndef __TIME_POSIX_H__
#define __TIME_POSIX_H__

#include "config.h"	// macros, bool, uint[XX]_t

#include <stdio.h>	// perror
#include <time.h>	// clock_gettime

static inline uint64_t get_time(void)
{
	struct timespec tv;

	if(unlikely(clock_gettime(CLOCK_MONOTONIC, &tv) == -1))
	{
		perror("clock_gettime");
		return 0;
	}

	return tv.tv_sec * 1000000000L + tv.tv_nsec;
}

#endif /*__TIME_POSIX_H__*/
