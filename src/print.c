#include "config.h"	// macros
#include <stdarg.h>	// required for gcc, because lol. (not clang/msvc)
#include <stdio.h>	// ?fprintf
#include <stdlib.h>	// exit

#include "sgherm.h"	// emu_state
#include "util.h"	// UNUSED


FILE *to_stdout;
FILE *to_stderr;

NORETURN void fatal(emu_state *state UNUSED, const char *str, ...)
{
	va_list argp;
	va_start(argp, str);

	fprintf(to_stderr, "FATAL ERROR during execution: ");
	vfprintf(to_stderr, str, argp);
	fprintf(to_stderr, "\n");

	va_end(argp);

	exit(EXIT_FAILURE);
}

void error(emu_state *state UNUSED, const char *str, ...)
{
	va_list argp;
	va_start(argp, str);

	fprintf(to_stderr, "ERROR during execution: ");
	vfprintf(to_stderr, str, argp);
	fprintf(to_stderr, "\n");

	va_end(argp);
}

void info(emu_state *state UNUSED, const char *str, ...)
{
	va_list argp;
	va_start(argp, str);

	fprintf(to_stderr, "info: ");
	vfprintf(to_stderr, str, argp);
	fprintf(to_stderr, "\n");

	va_end(argp);
}

void warning(emu_state *state UNUSED, const char *str, ...)
{
	va_list argp;
	va_start(argp, str);

	fprintf(to_stderr, "WARNING: ");
	vfprintf(to_stderr, str, argp);
	fprintf(to_stderr, "\n");

	va_end(argp);
}

#ifdef NDEBUG
void debug(emu_state *state UNUSED, const char *str UNUSED, ...)
{
	// Stub
}
#else
void debug(emu_state *state UNUSED, const char *str, ...)
{
	va_list argp;
	va_start(argp, str);

	vfprintf(to_stderr, str, argp);
	fprintf(to_stderr, "\n");

	va_end(argp);
}
#endif
