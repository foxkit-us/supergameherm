#include <stdbool.h>	// bool
#include <stdio.h>	// file methods
#include <stdlib.h>	// exit
#include <string.h>	// memset

#include "ctl_unit.h"	// init_ctl, execute
#include "lcdc.h"	// lcdc_tick
#include "memory.h"	/* offsets, emulator_state */
#include "params.h"	// system_types
#include "print.h"	// fatal, error, debug
#include "serio.h"	// serial_tick
#include "timer.h"	// init_clock

typedef void (*opcode_t)(emulator_state *state);
extern opcode_t *handlers;

uint8_t count_unimplemented(void)
{
	uint8_t c = 0;
	int i;

	for(i = 0x0; i < 0x100; i++)
	{
		if(handlers[i] == NULL)
		{
			c++;
		}
	}

	return c;
}

void init_emulator(emulator_state *state)
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

	printf("Unimplemented instructions: %d\n", count_unimplemented());

	init_ctl(&state, system);
	do
	{
		if((++cycles % 8400000) == 0) printf("GBC seconds: %ld\n", cycles / 8400000);
		execute(&state);
		lcdc_tick(&state);
		serial_tick(&state);
		timer_tick(&state);
		//clock_tick(&state);

	}
	while (true);

	return EXIT_SUCCESS;
}
