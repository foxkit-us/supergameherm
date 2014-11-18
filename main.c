#include <stdbool.h>	// bool
#include <stdio.h>	// file methods
#include <stdlib.h>	// exit
#include <string.h>	// memset

#include "params.h"	// system_types
#include "print.h"	// fatal, error, debug
#include "memory.h"	/* offsets, emulator_state */

void init_emulator(emulator_state *state)
{
	memset(state, 0, sizeof(emulator_state));
	state->interrupts = true;
}

int main(int argc, char *argv[])
{
	FILE *rom;
	emulator_state state;

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

	if(!read_rom_data(&state, rom))
	{
		fatal("can't read ROM data (ROM is corrupt)?");
	}

	fclose(rom);

	init_ctl(&state, SYSTEM_SGB);
	while(execute(&state));

	return EXIT_SUCCESS;
}
