#include "config.h"	// bool

#include "ctl_unit.h"	// init_ctl, execute
#include "debug.h"	// print_cycles
#include "frontend.h"	// null_frontend
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


emu_state * init_emulator(void)
{
	emu_state *state = (emu_state *)calloc(1, sizeof(emu_state));

	for(int i = 0; i < 0xF; i++)
	{
		state->cart_ram[i] = (unsigned char *)malloc(0x2000);
	}

	state->int_state.enabled = true;
	state->bank = 1;
	state->wait = 1;
	state->freq = CPU_FREQ_DMG;

	state->front = null_frontend;

	return state;
}

static void finish_emulator(emu_state *restrict state)
{
	free(state->cart_data);
	for(int i = 0xE; i >= 0; i--)
	{
		free(state->cart_ram[i]);
	}
	free(state);
}

int main(int argc, char *argv[])
{
	FILE *rom;
	emu_state *state;
	cart_header *header;
	uint32_t count_cur_second = 0, game_seconds = 0;
	int code = EXIT_SUCCESS;

	printf("Super Game Herm!\n");
	printf("Beta version!\n\n");

	printf("sizeof state: %zd\n", sizeof(emu_state));

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

	if(unlikely(!read_rom_data(state, rom, &header)))
	{
		fatal("can't read ROM data (ROM is corrupt)?");
		return EXIT_FAILURE;
	}

	fclose(rom);

	init_ctl(state);

	// Register the handlers
	register_handlers();

	// Set the starting clock
	state->start_time = get_time();

	do
	{
		int val = CALL_FRONTEND(state, event_loop);
		if(unlikely(val))
		{
			do_exit = true;
			code = val;
		}

		execute(state);
		lcdc_tick(state);
		serial_tick(state);
		timer_tick(state);
		sound_tick(state);
		//clock_tick(state);

		if(unlikely(++count_cur_second == state->freq))
		{
			count_cur_second = 0;
			if((++game_seconds % 10) == 0)
			{
				debug("GBC seconds: %ld", ++game_seconds);
			}
		}
	}
	while(likely(++state->cycles && !do_exit));

	finish_emulator(state);

	return code;
}
