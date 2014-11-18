#ifndef __TIMER_H_
#define __TIMER_H_

#include <stdio.h>	// perror
#include <time.h>	// nanosleep, timespec

#include "print.h"	// fatal
#include "sgherm.h"	// emulator_state

static inline void clock_tick(emulator_state *state)
{
	struct timespec req, _new;

	req.tv_sec = 0;
	req.tv_nsec = 230; // This isn't right but it'll do for now

	if(unlikely(clock_nanosleep(CLOCK_MONOTONIC_COARSE, 0, &req, &_new) == -1))
	{
		perror("clock_nanosleep");
		// What else can we do?
	}
}

#endif /*!__TIMER_H_*/
#warning this module is only tested on Linux; other platforms *badly* need testing

