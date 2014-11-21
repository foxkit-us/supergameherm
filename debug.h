#ifndef __DEBUG_H_
#define __DEBUG_H_

#include "config.h"	// Various macros

#include "sgherm.h"	// emu_state

void print_cpu_state(emu_state *restrict);
void print_cycles(emu_state *restrict);
void print_flags(emu_state *restrict);

#endif /*!__DEBUG_H_*/
