#ifndef __TIMER_H_
#define __TIMER_H_

#include "config.h"	// Various macros

#include <stdio.h>	// perror
#include <stdint.h>	// uint[XX]_t

#include "print.h"	// fatal
#include "util.h"	// likely/unlikely
#include "sgherm.h"	// emu_state


uint8_t timer_read(emu_state *restrict, uint16_t);
void timer_write(emu_state *restrict, uint16_t, uint8_t);
void timer_tick(emu_state *restrict);


#ifdef HAVE_CLOCK_GETTIME
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
#elif defined(HAVE_MACH_CLOCK_H)
#include <mach/clock.h>	// clock_serv_t, mach_timespec_t, etc.
#include <mach/mach.h>	// mach_port_deallocate

static inline uint64_t get_time(void)
{
	static float adj_const = 0.0F;

	// Cache the value (it doesn't change)
	if(adj_const == 0.0F)
	{
		mach_timebase_info_data_t ti;
		mach_timebase_info(&ti);

		adj_const = ti.numer / ti.denom;
	}

	return (uint64_t)mach_absolute_time() * adj_const;
}

#else
static inline uint64_t get_time(void)
{
	// FIXME Windows, OS X, etc.
	return 0;
}
#endif /*defined HAVE_CLOCK_GETTIME*/

#endif /*!__TIMER_H_*/
