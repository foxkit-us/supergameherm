#include "config.h"	// bool

#include "ctl_unit.h"	// init_ctl, execute
#include "debug.h"	// print_cycles
#include "frontend.h"	// null_frontend_*
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


emu_state * init_emulator(const char *rom_path)
{
	emu_state *state = (emu_state *)calloc(1, sizeof(emu_state));
	cart_header *header;
	FILE *rom;

	state->interrupts.enabled = true;
	state->bank = 1;
	state->wait = 1;
	state->freq = CPU_FREQ_DMG;

	state->front.input = null_frontend_input;
	state->front.audio = null_frontend_audio;
	state->front.video = null_frontend_video;
	state->front.event_loop = null_event_loop;

	if((rom = fopen(rom_path, "rb")) == NULL)
	{
		perror("fopen");
		free(state);
		state = NULL;
		goto end_init;
	}

	if(unlikely(!read_rom_data(state, rom, &header)))
	{
		fatal("can't read ROM data (ROM is corrupt)?");
		free(state);
		state = NULL;
		goto end_init;
	}

	// Initalise state
	init_ctl(state);

	// Start the clock
	state->start_time = get_time();

end_init:
	fclose(rom);

	return state;
}

static void finish_emulator(emu_state *restrict state)
{
	print_cycles(state);

	free(state->cart_data);
	free(state);
}

bool step_emulator(emu_state *restrict state)
{
	static uint32_t count_cur_second = 0, game_seconds = 0;

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

	state->cycles++;

	return true;
}

int main(int argc, char *argv[])
{
	emu_state *state;
	int val;

	printf("Super Game Herm!\n");
	printf("Beta version!\n\n");

	//printf("sizeof state: %zd\n", sizeof(emu_state));

	if(argc < 2)
	{
		fatal("You must specify a ROM file... -.-");
		return EXIT_FAILURE;
	}

	// Register the handlers
	register_handlers();

	if((state = init_emulator(argv[1])) == NULL)
	{
		fatal("Error initalising the emulator :(");
		return EXIT_FAILURE;
	}

	FRONTEND_INIT_ALL(state)
	val = EVENT_LOOP(state);

	FRONTEND_FINISH_ALL(state)
	finish_emulator(state);

	return val;
}
