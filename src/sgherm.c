#include "config.h"	// bool

#include "sgherm.h"	// emu_state, constants
#include "ctl_unit.h"	// init_ctl, execute
#include "lcdc.h"	// lcdc_tick
#include "sound.h"	// sound_tick
#include "timer.h"	// timer_tick
#include "serio.h"	// serial_tick
#include "debug.h"	// print_cycles
#include "print.h"	// fatal, error, debug
#include "util_time.h"	// get_time
#include "mbc.h"	// MBC_FINISH

#include <stdio.h>	// file methods
#include <stdlib.h>	// exit
#include <string.h>	// memset, strerror
#include <errno.h>	// errno
#include <assert.h>	// assert

// This is probably wrong but it seems to be smooth enough
#define NSEC_PER_VBLANK 16680567L

emu_state * init_emulator(const char *rom_path, const char *save_path)
{
	emu_state *state = (emu_state *)calloc(1, sizeof(emu_state));
	cart_header *header;

	if(strcmp(rom_path, save_path) == 0)
	{
		error(state, "ROM path can't be the same as the save path!");
		return NULL;
	}

	state->save_path = save_path;

	state->interrupts.enabled = true;
	state->wait = 1;
	state->freq = CPU_FREQ_DMG;

	if(unlikely(!read_rom_data(state, rom_path, &header)))
	{
		error(state, "can't read ROM data (ROM is corrupt)?");
		free(state);
		return NULL;
	}

	// Initalise state
	init_ctl(state);
	init_lcdc(state);

	// Start the clock
	state->start_time = get_time();

	return state;
}

void finish_emulator(emu_state *restrict state)
{
	print_cycles(state);

	MBC_FINISH(state);
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

	if(state->mbc.dirty && (state->cycles % state->freq) == 0)
	{
		// Do a write back
		memmap_sync(state, state->mbc.cart_ram, &(state->mbc.cart_mm_data));
	}

#ifdef THROTTLE_VBLANK
	// Wait for vblank
	if(unlikely(LCDC_STAT_MODE_FLAG(state) == 1 &&
		state->lcdc.curr_clk == 0))
	{
		uint64_t t = get_time();
		uint64_t wait = t - state->last_vblank_time;

		state->last_vblank_time = t;

		if(wait < NSEC_PER_VBLANK)
		{
			sleep_nsec(NSEC_PER_VBLANK - wait);
		}
	}
#endif

	return true;
}
