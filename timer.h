#ifndef __TIMER_H_
#define __TIMER_H_

#include "config.h"	// Various macros

#include <stdio.h>	// perror
#include <stdint.h>	// uint[XX]_t

#include "print.h"	// fatal
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
	clock_serv_t clock;
	mach_timespec_t mtv;

	host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &clock);
	clock_get_time(clock, &mtv);
	mach_port_deallocate(mach_task_self(), clock);

	return mtv.tv_sec * 1000000000L + mtv.tv_nsec;
}

#else
static inline uint64_t get_time(void)
{
	// FIXME Windows, OS X, etc.
	return 0;
}
#endif /*defined HAVE_CLOCK_GETTIME*/

#endif /*!__TIMER_H_*/
