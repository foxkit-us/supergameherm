#ifndef __TIME_NULL_H__
#define __TIME_NULL_H__

#include "config.h"	// macros, bool, uint[XX]_t

#include <time.h>

static inline uint64_t get_time(void)
{
	clock_t res = clock();
	uint64_t val = res * (1000000000L / CLOCKS_PER_SEC);
	return val;
}

#endif /*__TIME_NULL_H__*/
