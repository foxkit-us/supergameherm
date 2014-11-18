#warning this module is only tested on Linux; other platforms *badly* need testing

#include <stdio.h>	// perror
#include <time.h>	// nanosleep, timespec

#include "print.h"	// fatal
#include "sgherm.h"	// emulator_state


bool clock_tick(emulator_state *state)
{
	struct timespec req, new;

	req.tv_sec = 0;
	req.tv_nsec = 2;

	/*if(nanosleep(&req, &new))
	{
		perror("nanosleep");
		return false;
	}*/

	return true;
}