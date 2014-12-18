#ifndef __SLEEP_NULL_H__
#define __SLEEP_NULL_H__

#include "config.h"	// macros, bool, uint[XX]_t, get_time

static inline void sleep_nsec(uint64_t nsec)
{
	// Crappy busy-waiting implementation
	uint64_t start = get_time();

	if(start == 0)
	{
		// Umm... yeah I dunno what to do, sorry!
		return;
	}

	while((get_time() - start) < nsec);
}

#endif /*__SLEEP_NULL_H__*/
