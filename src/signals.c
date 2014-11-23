#include "config.h"	// macros

#include <stdlib.h>	// atexit

#include "print.h"	// error
#include "debug.h"	// print_cycles
#include "util.h"	// unused
#include "sgherm.h"	// emu_state


volatile bool do_exit = false;

emu_state *state_current; // XXX should be a linked list or such


void exit_print_stats(void)
{
	print_cycles(state_current);
}


#ifdef HAVE_POSIX

#include <signal.h>	// sigaction

static void sig_handler(int signal unused)
{
	do_exit = true;
}

void register_handlers(void)
{
	struct sigaction sa;

	sigemptyset(&sa.sa_mask);
	sa.sa_handler = &sig_handler;
	sa.sa_flags = 0;
	if (sigaction(SIGHUP, &sa, NULL) ||
		sigaction(SIGINT, &sa, NULL) ||
		sigaction(SIGQUIT, &sa, NULL) ||
		sigaction(SIGTERM, &sa, NULL) ||
#ifdef SIGXCPU
		sigaction(SIGXCPU, &sa, NULL) ||
#endif
#ifdef SIGXFSZ
		sigaction(SIGXFSZ, &sa, NULL) ||
#endif
#ifdef SIGIO
		sigaction(SIGIO, &sa, NULL) ||
#endif
		sigaction(SIGPIPE, &sa, NULL) ||
		sigaction(SIGALRM, &sa, NULL) ||
		atexit(exit_print_stats))
	{
		error("Could not initalise signal handlers, possibly no stats printing :(");
	}
}

#else /*HAVE_POSIX */

void register_handlers(void)
{
	if(atexit(exit_print_stats))
	{
		error("Could not initalise signal handlers, possibly no stats printing :(");
	}
}

#endif /*HAVE_POSIX */
