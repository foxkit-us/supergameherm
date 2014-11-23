#include "config.h"	// macros

#include <stdbool.h>	// bool
#include <stdio.h>	// file methods
#include <stdlib.h>	// exit
#include <string.h>	// memset

#include "ctl_unit.h"	// init_ctl, execute
#include "lcdc.h"	// lcdc_tick
#include "sgherm.h"	// emu_state
#include "rom_read.h"	// offsets
#include "params.h"	// system_types
#include "print.h"	// fatal, error, debug
#include "serio.h"	// serial_tick
#include "timer.h"	// get_clock
#include "debug.h"	// print_cycles
#include "signals.h"	// register_handler
#include "sound.h"	// sound_tick
#include "util.h"	// likely/unlikely

emu_state * init_emulator(void)
{
	emu_state *state = (emu_state *)malloc(sizeof(emu_state));
	emu_state state2 =
	{
		.registers = {
			.a = ((uint8_t *)&(state->registers.af)) + 1,
			.f = (uint8_t *)&(state->registers.af),
			.b = ((uint8_t *)&(state->registers.bc)) + 1,
			.c = (uint8_t *)&(state->registers.bc),
			.d = ((uint8_t *)&(state->registers.de)) + 1,
			.e = (uint8_t *)&(state->registers.de),
			.h = ((uint8_t *)&(state->registers.hl)) + 1,
			.l = (uint8_t *)&(state->registers.hl),
			.interrupts = true,
		},
		.bank = 1,
		.wait = 1,
		.freq = CPU_FREQ_GB,
	};

	memcpy(state, &state2, sizeof(emu_state));

	return state;
}

static void finish_emulator(emu_state *restrict state)
{
	free(state->cart_data);
	free(state);
}

int main(int argc, char *argv[])
{
	FILE *rom;
	system_types system;
	emu_state *state;
	cart_header *header;
	uint32_t count_cur_second = 0, gbc_seconds = 0;

	printf("Super Game Herm!\n");
	printf("Beta version!\n\n");

	if(argc < 2)
	{
		fatal("You must specify a ROM file... -.-");
		return EXIT_FAILURE;
	}

	if(unlikely((state_current = state = init_emulator()) == NULL))
	{
		fatal("Out of memory :(");
		return EXIT_FAILURE;
	}

	if(unlikely((rom = fopen(argv[1], "rb")) == NULL))
	{
		perror("open rom");
		fatal("Can't open ROM file %s", argv[1]);
		return EXIT_FAILURE;
	}

	if(unlikely(!read_rom_data(state, rom, &header, &system)))
	{
		fatal("can't read ROM data (ROM is corrupt)?");
		return EXIT_FAILURE;
	}

	fclose(rom);

	init_ctl(state, system);

	// Register the handlers
	register_handlers();

	// Set the starting clock
	state->start_time = get_time();

	do
	{
		execute(state);
		lcdc_tick(state);
		serial_tick(state);
		timer_tick(state);
		sound_tick(state);
		//clock_tick(state);

		if(unlikely(++count_cur_second == state->freq))
		{
			count_cur_second = 0;
			debug("GBC seconds: %ld", ++gbc_seconds);
		}
	}
	while(likely(++state->cycles && !do_exit));

	finish_emulator(state);

	return EXIT_SUCCESS;
}
