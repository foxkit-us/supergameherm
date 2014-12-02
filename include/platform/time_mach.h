#ifndef __TIME_MACH_H__
#define __TIME_MACH_H__

#include "config.h"	// macros, bool, uint[XX]_t

#include <mach/clock.h>	// clock_serv_t, mach_timespec_t, etc.
#include <mach/mach.h>	// mach_port_deallocate
#include <mach/mach_time.h>

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

#endif /*__TIME_MACH_H__*/
