#ifndef __SLEEP_POSIX_H__
#define __SLEEP_POSIX_H__

#include "config.h"	// macros, bool, uint[XX]_t

#include <stdio.h>	// perror
#include <time.h>	// nanosleep
#include <errno.h>	// errno

#define NSEC_SEC 1000000000L

static inline void sleep_nsec(uint64_t nsec)
{
	struct timespec ts, ret;

	ts.tv_sec = nsec / NSEC_SEC;
	ts.tv_nsec = nsec % NSEC_SEC;

	if(nanosleep(&ts, &ret))
	{
		switch(errno)
		{
		case EAGAIN:
		case EINTR:
		{
			uint64_t val;
			ts.tv_sec -= ret.tv_sec;
			ts.tv_nsec -= ret.tv_nsec;
			val = ts.tv_sec * NSEC_SEC + ts.tv_nsec;

			if(val < nsec)
			{
				sleep_nsec(ts.tv_sec * NSEC_SEC + ts.tv_nsec);
			}
			return;
		}
		default:
			perror("nanosleep");
			return;
		}
	}
}


#endif /*__SLEEP_POSIX_H__*/
