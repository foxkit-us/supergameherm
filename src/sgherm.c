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


#define NSEC_PER_SECOND 1000000000L
#define NSEC_PER_VBLANK NSEC_PER_SECOND / 60


emu_state * init_emulator(const char *bootrom_path, const char *rom_path, const char *save_path)
{
	emu_state *state = (emu_state *)calloc(1, sizeof(emu_state));
	cart_header *header;

	if(!rom_path)
	{
		error(state, "Unspecified ROM path!");
		return NULL;
	}

	if(save_path && strcmp(rom_path, save_path) == 0)
	{
		error(state, "Save path can't be the same as ROM path (ignoring)");
		save_path = NULL;
	}

	if(bootrom_path && ((save_path && strcmp(bootrom_path, save_path) == 0) ||
		strcmp(bootrom_path, rom_path) == 0))
	{
		warning(state, "Boot ROM path cannot be same as ROM path or save path (ignoring)");
		bootrom_path = NULL;
	}

	state->save_path = save_path ? strdup(save_path) : NULL;

	state->interrupts.enabled = true;
	state->wait = 1;
	state->freq = CPU_FREQ_DMG;

	if(unlikely(!read_rom_data(state, rom_path, &header)))
	{
		error(state, "Can't read ROM data (ROM is corrupt)?");
		free(state);
		return NULL;
	}

	if(bootrom_path)
	{
		if(!read_bootrom_data(state, bootrom_path))
		{
			warning(state, "Can't read boot ROM data, continuing without it");
		}
		else
		{
			state->in_bootrom = true;
		}
	}

	// Initalise state
	init_ctl(state);
	init_lcdc(state);

	// Start the clock
	state->start_time = get_time();
	state->next_vblank_time = state->start_time + NSEC_PER_VBLANK;

	return state;
}

void finish_emulator(emu_state *restrict state)
{
	print_cycles(state);

	MBC_FINISH(state);

	free(state->cart_data);
	if(state->save_path != NULL)
	{
		free((void *)state->save_path);
	}
	free(state);
}

bool step_emulator(emu_state *restrict state)
{
	// TODO: handle CGB speed better
	int count_per_step = 1;
	int count_per_step_core = (state->freq == CPU_FREQ_CGB ? 2 : 1);

	// The overhead of calling execute is enough where this is worthwhile
	if(state->wait)
	{
		state->wait--;
	}
	else
	{
		execute(state, count_per_step_core);
	}

	lcdc_tick(state, count_per_step);
	serial_tick(state, count_per_step_core);
	timer_tick(state, count_per_step_core);
	sound_tick(state, count_per_step);

	state->cycles += count_per_step_core;

	if(unlikely(state->mbc.dirty && ++(state->mbc.dirty_timer) == state->freq))
	{
		// Do a write back
		memmap_sync(state, state->mbc.cart_ram, &(state->mbc.cart_mm_data));
		state->mbc.dirty = false;
		state->mbc.dirty_timer = 0;
	}

#ifdef THROTTLE_VBLANK
	// Wait for vblank
	//if(unlikely(LCDC_STAT_MODE_FLAG(state) == 1 &&
	//	state->lcdc.curr_clk == 0))
	if(unlikely(state->lcdc.throt_trigger))
	{
		state->lcdc.throt_trigger = false;
		uint64_t t = get_time();
		int64_t wait = (int64_t)state->next_vblank_time - (int64_t)t;

		if(wait > 1000000)
		{
			sleep_nsec(wait);
		}

		state->next_vblank_time += (int64_t)NSEC_PER_VBLANK;

		// Ensure we don't throttle too much
		if(state->next_vblank_time + (int64_t)(NSEC_PER_VBLANK*5) < t)
		{
			printf("Throttle reduced\n");
			state->next_vblank_time = t - (int64_t)(NSEC_PER_VBLANK*5);
		}
	}
#endif //THROTTLE_VBLANK

	return true;
}
