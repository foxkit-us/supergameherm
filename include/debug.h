#ifndef __DEBUG_H_
#define __DEBUG_H_

#include "config.h"	// Various macros

#include "sgherm.h"	// emu_state

extern const char * const mnemonics[0x100];
extern const int instr_len[0x100];

void print_cpu_state(emu_state *restrict);
void print_cycles(emu_state *restrict);
void print_flags(emu_state *restrict);
void dump_all_state(emu_state *restrict);

#endif /*!__DEBUG_H_*/
