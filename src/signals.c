#include "config.h"	// macros

#include "print.h"	// error
#include "debug.h"	// print_cycles
#include "util.h"	// unused
#include "sgherm.h"	// emu_state


volatile bool do_exit = false;

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
		sigaction(SIGALRM, &sa, NULL))
	{
		error("Could not initalise signal handlers, possibly no stats printing :(");
	}
}

#else /*HAVE_POSIX */

void register_handlers(void)
{
	// TODO maybe some windows signal handling?
}

#endif /*HAVE_POSIX */
