#ifndef __TIMER_H_
#define __TIMER_H_

#include "config.h"	// Various macros

#include <stdio.h>	// perror
#include <stdint.h>	// uint[XX]_t

#include "print.h"	// fatal
#include "sgherm.h"	// emulator_state


uint8_t timer_read(emulator_state *restrict, uint16_t);
void timer_write(emulator_state *restrict, uint16_t, uint8_t);
void timer_tick(emulator_state *restrict);


#if defined HAVE_CLOCK_GETTIME
#include <time.h>	// clock_gettime

static inline uint64_t get_time(void)
{
	struct timespec tv;

	if(unlikely(clock_gettime(CLOCK_MONOTONIC, &tv) == -1))
	{
		perror("clock_gettime");
		return 0;
	}

	return tv.tv_sec * 1e9 + tv.tv_nsec;
}
#else
static inline uint64_t get_time(void)
{
	// FIXME Windows, OS X, etc.
	return 0;
}
#endif /*defined HAVE_CLOCK_GETTIME*/

#endif /*!__TIMER_H_*/
