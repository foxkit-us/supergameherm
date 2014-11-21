#include "config.h"	// macros

#include "timer.h"	// get_time
#include "sgherm.h"	// emu_state, cpu_freq
#include "print.h"	// debug
#include "ctl_unit.h"	// flags
#include "debug.h"	// Prototypes etc

void print_cpu_state(emu_state *restrict state)
{
	debug("[%X] (af bc de hl sp %X %X %X %X %X)", state->registers.pc,
		state->registers.af, state->registers.bc, state->registers.de,
		state->registers.hl, state->registers.sp);
}

void print_cycles(emu_state *restrict state)
{
	uint64_t finish = get_time();
	double taken = (finish - state->start_time) / 1e9;
	double cps = state->cycles / taken;
	const cpu_freq freq_gb = CPU_FREQ_GB, freq_gbc = CPU_FREQ_GBC;

	info("Time taken: %.3f seconds", taken);
	info("Cycle count: %ld", state->cycles);
	info("Cycles per second: %.3f (%.3fx GB, %.3fx GBC)", cps,
			cps / freq_gb, cps / freq_gbc);
}

void print_flags(emu_state *restrict state)
{
	debug("flags = %s%s%s%s",
		(*(state->registers.f) & FLAG_Z) ? "Z":"z",
		(*(state->registers.f) & FLAG_N) ? "N":"n",
		(*(state->registers.f) & FLAG_H) ? "H":"h",
		(*(state->registers.f) & FLAG_C) ? "C":"c");
}

void dump_all_state(emu_state *restrict state)
{
	debug("pc=%04X\tsp=%04X\tbk=%04X\n",
	       state->registers.pc,
	state->registers.sp,
	state->bank);
	debug("af=%04X\tbc=%04X\tde=%04X\thl=%04X\n",
	       state->registers.af,
	state->registers.bc,
	state->registers.de,
	state->registers.hl);
	debug("interrupts are %s\n", (state->iflags ? "ENABLED" : "DISABLED"));
	print_flags(state);
	debug("bytes at pc: %02X %02X %02X %02X\n",
	       mem_read8(state, state->registers.pc),
	       mem_read8(state, state->registers.pc+1),
	       mem_read8(state, state->registers.pc+2),
	       mem_read8(state, state->registers.pc+3));
	debug("bytes at sp: %02X %02X %02X %02X\n",
	       mem_read8(state, state->registers.sp),
	       mem_read8(state, state->registers.sp+1),
	       mem_read8(state, state->registers.sp+2),
	       mem_read8(state, state->registers.sp+3));
}
