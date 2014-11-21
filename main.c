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
#include "timer.h"	// init_clock

void init_emulator(emulator_state *restrict state)
{
	memset(state, 0, sizeof(emulator_state));
	state->interrupts = true;
	state->bank = 1;
}

int main(int argc, char *argv[])
{
	FILE *rom;
	system_types system;
	emulator_state state;
	cart_header *header;
	uint64_t cycles = 0;

	printf("Super Game Herm!\n");
	printf("Beta version!\n\n");

	if(argc < 2)
	{
		fatal("You must specify a ROM file... -.-");
	}

	init_emulator(&state);

	if(unlikely((rom = fopen(argv[1], "rb")) == NULL))
	{
		perror("open rom");
		fatal("Can't open ROM file %s", argv[1]);
	}

	if(unlikely(!read_rom_data(&state, rom, &header, &system)))
	{
		fatal("can't read ROM data (ROM is corrupt)?");
	}

	fclose(rom);

	init_ctl(&state, system);

	do
	{
		if((++cycles % 8400000) == 0)
		{
			debug("GBC seconds: %ld", cycles / 8400000);
			if((cycles % (8400000*60)) == 0)
			{
				break;
			}
		}

		execute(&state);
		lcdc_tick(&state);
		serial_tick(&state);
		timer_tick(&state);
		//clock_tick(&state);

	}
	while(likely(true));

	return EXIT_SUCCESS;
}
