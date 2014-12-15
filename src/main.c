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

emu_state * init_emulator(const char *rom_path, frontend_type input,
		frontend_type audio, frontend_type video,
		frontend_type event_loop)
{
	emu_state *state = (emu_state *)calloc(1, sizeof(emu_state));
	cart_header *header;
	FILE *rom;

	if((rom = fopen(rom_path, "rb")) == NULL)
	{
		perror("fopen");
		free(state);
		return NULL;
	}

	state->interrupts.enabled = true;
	state->bank = 1;
	state->wait = 1;
	state->freq = CPU_FREQ_DMG;

	memcpy(&(state->front.input), frontend_set_input[input], sizeof(frontend_input));
	memcpy(&(state->front.audio), frontend_set_audio[audio], sizeof(frontend_audio));
	memcpy(&(state->front.video), frontend_set_video[video], sizeof(frontend_video));
	state->front.event_loop = frontend_set_event_loop[event_loop];


	if(unlikely(!read_rom_data(state, rom, &header)))
	{
		fatal(state, "can't read ROM data (ROM is corrupt)?");
		free(state);
		fclose(rom);
		return NULL;
	}

	// Initalise state
	init_ctl(state);
	init_lcdc(state);

	// Start the clock
	state->start_time = get_time();

	fclose(rom);

	return state;
}

void finish_emulator(emu_state *restrict state)
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
			debug(state, "GBC seconds: %ld", ++game_seconds);
		}
	}

	state->cycles++;

	return true;
}

int main_common(emu_state *state)
{
	register_handlers();

	FRONTEND_INIT_ALL(state)
	int val = EVENT_LOOP(state);

	FRONTEND_FINISH_ALL(state)
	finish_emulator(state);

	return val;
}

int main(int argc, char *argv[])
{
	emu_state *state;

	to_stdout = stdout;
	to_stderr = stderr;

	fprintf(to_stdout, "Super Game Herm!\n");
	fprintf(to_stdout, "Beta version!\n\n");

	if(argc < 2)
	{
		fatal(NULL, "You must specify a ROM file... -.-");
		return EXIT_FAILURE;
	}

	state = init_emulator(argv[1], FRONT_SDL2, FRONT_SDL2, FRONT_SDL2, FRONT_SDL2);
	//state = init_emulator(argv[1], FRONT_LIBCACA, FRONT_NULL, FRONT_LIBCACA, FRONT_LIBCACA);
	if(state == NULL)
	{
		fatal(NULL, "Error initalising the emulator :(");
		return EXIT_FAILURE;
	}

	return main_common(state);
}
