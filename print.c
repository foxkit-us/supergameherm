#include <stdarg.h>	// required for gcc, because lol. (not clang/msvc)
#include <stdio.h>	// ?fprintf
#include <stdlib.h>	// exit


void fatal(const char *str, ...)
{
	va_list argp;
	va_start(argp, str);

	fprintf(stderr, "FATAL ERROR during execution: ");
	vfprintf(stderr, str, argp);
	fprintf(stderr, "\n");

	va_end(argp);

	exit(EXIT_FAILURE);
	/* NOTREACHED */
}

void error(const char *str, ...)
{
	va_list argp;
	va_start(argp, str);

	fprintf(stderr, "ERROR during execution: ");
	vfprintf(stderr, str, argp);
	fprintf(stderr, "\n");

	va_end(argp);
}

void info(const char *str, ...)
{
	va_list argp;
	va_start(argp, str);

	fprintf(stderr, "info: ");
	vfprintf(stderr, str, argp);
	fprintf(stderr, "\n");

	va_end(argp);
}

void debug(const char *str, ...)
{
#ifndef NDEBUG
	va_list argp;
	va_start(argp, str);

	vfprintf(stderr, str, argp);
	fprintf(stderr, "\n");

	va_end(argp);
#endif
}
