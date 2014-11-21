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

// XXX FIXME UGH
static emu_state *state_current;

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
		},
		.iflags = I_INTERRUPTS,
		.bank = 1,
		.freq = CPU_FREQ_GB,
	};

	memcpy(state, &state2, sizeof(emu_state));

	return state;
}

void finish_emulator(emu_state *restrict state)
{
	free(state->cart_data);
	free(state);
}

void exit_print_stats(void)
{
	print_cycles(state_current);
}

int main(int argc, char *argv[])
{
	FILE *rom;
	system_types system;
	emu_state *state;
	cart_header *header;
	uint32_t gbc_seconds = 0;

	printf("Super Game Herm!\n");
	printf("Beta version!\n\n");

	if(argc < 2)
	{
		fatal("You must specify a ROM file... -.-");
	}

	if(unlikely((state_current = state = init_emulator()) == NULL))
	{
		fatal("Out of memory :(");
	}

	if(unlikely((rom = fopen(argv[1], "rb")) == NULL))
	{
		perror("open rom");
		fatal("Can't open ROM file %s", argv[1]);
	}

	if(unlikely(!read_rom_data(state, rom, &header, &system)))
	{
		fatal("can't read ROM data (ROM is corrupt)?");
	}

	fclose(rom);

	init_ctl(state, system);

	// Set the starting clock
	state->start_time = get_time();

	atexit(exit_print_stats);

	do
	{
		if(unlikely(!(++state->cycles % state->freq)))
		{
			debug("GBC seconds: %ld", ++gbc_seconds);
		}
		execute(state);
		lcdc_tick(state);
		serial_tick(state);
		timer_tick(state);
		//clock_tick(state);
	}
	while(true);

	finish_emulator(state);

	return EXIT_SUCCESS;
}
