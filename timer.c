#warning this module is only tested on Linux; other platforms *badly* need testing

#include <sys/timerfd.h> // clock_*
#include <sys/types.h>	// FD_SET etc
#include <stdio.h>	// perror
#include <unistd.h>	// read

#include "print.h"	// fatal
#include "sgherm.h"	// emulator_state


void init_clock(emulator_state *state)
{
	struct itimerspec spec;
	struct timespec now;
	int fd;

	if(clock_gettime(CLOCK_MONOTONIC, &now) == -1)
	{
		perror("clock_gettime");
		fatal("couldn't get the time");
	}

	spec.it_value.tv_sec = now.tv_sec;
	spec.it_value.tv_nsec = now.tv_nsec + 238;
	spec.it_interval.tv_sec = 0;
	spec.it_interval.tv_nsec = 238;

	fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
	if(fd == -1)
	{
		perror("timerfd_create");
		fatal("upgrade your kernel (2.6.27+ required)");
	}

	if(timerfd_settime(fd, TFD_TIMER_ABSTIME, &spec, NULL) == -1)
	{
		perror("timerfd_settime");
		fatal("couldn't set the timer spec");
	}

	state->timer_obj = (void *)fd;

	return;
}

void clock_tick(emulator_state *state)
{
	fd_set rfds;
	int fd = (int)state->timer_obj;

	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);

	select(fd+1, &rfds, NULL, NULL, NULL);

	if(!FD_ISSET(fd, &rfds)) { debug("select failed?"); }
	else {
		uint64_t s;
		if(read(fd, &s, sizeof(uint64_t)) != sizeof(uint64_t))
		{
			debug("read failed");
		}
	}

	return;
}