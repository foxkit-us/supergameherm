#include "config.h"	// bool

#include "ctl_unit.h"	// init_ctl, execute
#include "debug.h"	// print_cycles
#include "frontend.h"	// null_frontend_*
#include "lcdc.h"	// lcdc_tick
#include "print.h"	// fatal, error, debug
#include "rom_read.h"	// offsets
#include "serio.h"	// serial_tick
#include "sgherm.h"	// emu_state, constants
#include "signals.h"	// register_handler
#include "sound.h"	// sound_tick
#include "timer.h"	// get_clock
#include "util_time.h"	// get_time

#include <stdio.h>	// file methods
#include <stdlib.h>	// exit
#include <string.h>	// memset


emu_state * init_emulator(const char *rom_path)
{
	emu_state *state = (emu_state *)calloc(1, sizeof(emu_state));
	cart_header *header;
	FILE *rom;

	if((rom = fopen(rom_path, "rb")) == NULL)
	{
		perror("fopen");
		free(state);
		return NULL;
	}

	state->interrupts.enabled = true;
	state->bank = 1;
	state->wait = 1;
	state->freq = CPU_FREQ_DMG;

	if(unlikely(!read_rom_data(state, rom, &header)))
	{
		fatal(state, "can't read ROM data (ROM is corrupt)?");
		free(state);
		fclose(rom);
		return NULL;
	}

	// Initalise state
	init_ctl(state);
	init_lcdc(state);

	// Start the clock
	state->start_time = get_time();

	fclose(rom);

	return state;
}

void finish_emulator(emu_state *restrict state)
{
	print_cycles(state);

	free(state->cart_data);
	free(state);
}

bool step_emulator(emu_state *restrict state)
{
	execute(state);
	lcdc_tick(state);
	serial_tick(state);
	timer_tick(state);
	sound_tick(state);

	state->cycles++;

	return true;
}
