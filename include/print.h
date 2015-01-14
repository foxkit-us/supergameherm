#ifndef __PRINT_H_
#define __PRINT_H_

#include "config.h"	// config
#include "sgherm.h"	// emu_state

#include <stdio.h>	// FILE *

/*!
 * @brief	Display an error and quit fast.
 * @param	state	The state raising the error.  NULL if global.
 * @param	str	The format of the error to print.
 * @result	The error is printed and the program is immediately
 * 		terminated with an error exit code.
 * 		(atexit() methods are still run - use abort if you don't
 * 		like it.)
 */
NORETURN void fatal(emu_state *, const char *, ...);

/*!
 * @brief	Report an error condition to the user.
 * @param	state	The state raising the error.  NULL if global.
 * @param	str	The format of the error to print.
 */
void error(emu_state *, const char *, ...);

/*!
 * @brief	Display information to the user.
 * @param	state	The state showing the message.  NULL if global.
 * @param	str	The format of the information to print.
 */
void info(emu_state *, const char *, ...);

/*!
 * @brief	Report a warning condition to the user
 * @param	state	The state reporting the warning.  NULL if global.
 * @param	str	The format of the warning to print
 */
void warning(emu_state *, const char *, ...);

/*!
 * @brief	Display debug information to the user.
 * @param	state	The state being debugged.  NULL if global.
 * @param	str	The format of the information to print.
 * @note	This method is a no-op if NDEBUG is defined during
 * 		compilation.  Do not rely on this method for important
 * 		messages; use info instead.
 */
void debug(emu_state *, const char *, ...);

//! Where stdout goes with these functions
extern FILE *to_stdout;

//! Where stderr goes with these functions
extern FILE *to_stderr;

#endif /*!__PRINT_H_*/
