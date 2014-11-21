#include "config.h"	// macros

#include "timer.h"	// get_time
#include "sgherm.h"	// emulator_state
#include "print.h"	// debug
#include "ctl_unit.h"	// flags
#include "debug.h"	// Prototypes etc

void print_cpu_state(emulator_state *restrict state)
{
	debug("[%X] (af bc de hl sp %X %X %X %X %X)", state->registers.pc,
		state->registers.af, state->registers.bc, state->registers.de,
		state->registers.hl, state->registers.sp);
}

void print_cycles(emulator_state *restrict state)
{
	uint64_t finish = get_time();
	double taken = (finish - state->start_time) / 1e9;
	double cps = state->cycles / taken;

	debug("Time taken: %.3f seconds", taken);
	debug("Cycle count: %ld", state->cycles);
	debug("Cycles per second: %.3f (%.3f CGB's)", cps, cps / state->freq);
}

void print_flags(emulator_state *restrict state)
{
	debug("flags = %s%s%s%s",
		(*(state->registers.f) & FLAG_Z) ? "Z":"z",
		(*(state->registers.f) & FLAG_N) ? "N":"n",
		(*(state->registers.f) & FLAG_H) ? "H":"h",
		(*(state->registers.f) & FLAG_C) ? "C":"c");
}
