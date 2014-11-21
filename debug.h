#ifndef __DEBUG_H_
#define __DEBUG_H_

#include "config.h"	// Various macros

#include "sgherm.h"	// emulator_state

void print_cpu_state(emulator_state *restrict);
void print_cycles(emulator_state *restrict);
void print_flags(emulator_state *restrict);

#endif /*!__DEBUG_H_*/
