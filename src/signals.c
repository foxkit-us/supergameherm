#include "config.h"	// macros

#include "print.h"	// error
#include "debug.h"	// print_cycles
#include "util.h"	// UNUSED
#include "sgherm.h"	// emu_state


volatile bool do_exit = false;

#ifdef HAVE_POSIX

#include <signal.h>	// sigaction

static void sig_handler(int signal UNUSED)
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

#elif defined(_WIN32)

#undef UNUSED	// windows.h *chokes* on this
#include <windows.h>

/*!
 * @brief Handle Windows control events.
 * @param	type	The type of control event that has occurred.
 * @result TRUE if we handled it ourselves (and will go down within 20 secs),
 *         FALSE if we want Windows to kill us immediately.
 */
BOOL WINAPI ctrl_event_handler(DWORD type)
{
	switch(type)
	{
	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT:
	default:
		do_exit = true;
		return TRUE;
	case CTRL_CLOSE_EVENT:
		// console window is closed
		// note that from here ALL events mean the console is destroyed
		// so we CANNOT output to it (could crash/hang.)
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
		// just go down fast.
		return FALSE;
	}
}

void register_handlers(void)
{
	SetConsoleCtrlHandler(ctrl_event_handler, TRUE);
}

#else /* !HAVE_POSIX, !_WIN32 */

void register_handlers(void)
{
	// TODO maybe some windows signal handling?
}

#endif /* HAVE_POSIX */
