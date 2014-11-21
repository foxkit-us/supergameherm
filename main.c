#include "config.h"	// macros

#include <stdbool.h>	// bool
#include <stdio.h>	// file methods
#include <stdlib.h>	// exit
#include <string.h>	// memset

#include "ctl_unit.h"	// init_ctl, execute
#include "lcdc.h"	// lcdc_tick
#include "sgherm.h"	// emulator_state
#include "rom_read.h"	// offsets
#include "params.h"	// system_types
#include "print.h"	// fatal, error, debug
#include "serio.h"	// serial_tick
#include "timer.h"	// get_clock
#include "debug.h"	// print_cycles

// XXX FIXME UGH
static emulator_state *state_current;

emulator_state * init_emulator(void)
{
	emulator_state *state = calloc(sizeof(emulator_state), 1);
	emulator_state state2 =
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

	memcpy(state, &state2, sizeof(emulator_state));

	return state;
}

void exit_print_stats(void)
{
	print_cycles(state_current);
}

int main(int argc, char *argv[])
{
	FILE *rom;
	system_types system;
	emulator_state *state;
	cart_header *header;

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
		if((++state->cycles % state->freq) == 0)
		{
			debug("GBC seconds: %ld", state->cycles / state->freq);
			if((state->cycles % (state->freq*60)) == 0)
			{
				break;
			}
		}

		execute(state);
		lcdc_tick(state);
		serial_tick(state);
		timer_tick(state);
		//clock_tick(state);
	}
	while(likely(true));

	free(state);

	return EXIT_SUCCESS;
}
