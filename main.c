#include <stdbool.h>	// bool
#include <stdio.h>	// file methods
#include <stdlib.h>	// exit
#include <string.h>	// memset

#include "ctl_unit.h"	// init_ctl, execute
#include "params.h"	// system_types
#include "print.h"	// fatal, error, debug
#include "memory.h"	/* offsets, emulator_state */
#include "timer.h"	// init_clock

void init_emulator(emulator_state *state)
{
	memset(state, 0, sizeof(emulator_state));
	state->interrupts = true;
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

	rom = fopen(argv[1], "rb");
	if(rom == NULL)
	{
		perror("open rom");
		fatal("Can't open ROM file %s", argv[1]);
	}

	if(!read_rom_data(&state, rom, &header, &system))
	{
		fatal("can't read ROM data (ROM is corrupt)?");
	}

	fclose(rom);

	init_ctl(&state, system);
	do
	{
		if((++cycles % 8400000) == 0) printf("GBC seconds: %ld\n", cycles / 8400000);
		execute(&state);
		lcdc_tick(&state);
		//clock_tick(&state);

	} while (true);

	return EXIT_SUCCESS;
}
