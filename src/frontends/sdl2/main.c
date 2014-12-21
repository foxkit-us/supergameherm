#include "config.h"	// bool

#include "sgherm.h"	// *_emulator
#include "print.h"	// to_stdout, to_stderr
#include "signals.h"	// register_handlers

#include <SDL.h>	// main
#include <stdio.h>	// file methods
#include <stdlib.h>	// exit
#include <string.h>	// memset


int main(int argc, char *argv[])
{
	emu_state *state;
	int val;

	register_handlers();

	to_stdout = stdout;
	to_stderr = stderr;

	fprintf(to_stdout, "Super Game Herm (SDL2 frontend)!\n");
	fprintf(to_stdout, "Beta version!\n\n");

	if(argc < 2)
	{
		fatal(NULL, "You must specify a ROM file... -.-");
		return EXIT_FAILURE;
	}

	if((state = init_emulator(argv[1])) == NULL)
	{
		fatal(NULL, "Error initalising the emulator :(");
		return EXIT_FAILURE;
	}

	if(!select_frontend_all(state, SDL2_INPUT, SDL2_AUDIO, SDL2_VIDEO,
				SDL2_LOOP))
	{
		fatal(state, "Could not initalise SDL2 frontend");
		return EXIT_FAILURE;
	}

	if((val = EVENT_LOOP(state)))
	{
		fatal(state, "Emulator exited abnormally");
	}

	finish_frontend(state);
	finish_emulator(state);

	return val;

}
