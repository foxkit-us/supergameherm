#include "config.h"	// bool

#include "sgherm.h"	// *_emulator
#include "print.h"	// to_std*
#include "signals.h"	// register_handlers

#include <stdio.h>	// file methods
#include <stdlib.h>	// exit
#include <string.h>	// memset


int main(int argc, char *argv[])
{
	emu_state *state;
	const char *save = NULL;
	const char *bootrom = NULL;
	int val;

	register_handlers();

	to_stdout = stdout;
	to_stderr = freopen("stderr.log", "a", stderr);

	fprintf(to_stdout, "Super Game Herm (null frontend)!\n");
	fprintf(to_stdout, "Beta version!\n\n");

	// FIXME use getopt
	if(argc < 2)
	{
		fatal(NULL, "You must specify a ROM file... -.-");
		return EXIT_FAILURE;
	}
	else if(argc == 3)
	{
		save = argv[2];
	}
	else if(argc >= 4)
	{
		bootrom = argv[3];
	}

	if((state = init_emulator(bootrom, argv[1], save)) == NULL)
	{
		fatal(NULL, "Error initalising the emulator :(");
		return EXIT_FAILURE;
	}

	// This never fails for the NULL frontend
	select_frontend_all(state, NULL_AUDIO, NULL_VIDEO, NULL_LOOP);

	if((val = EVENT_LOOP(state)))
	{
		fatal(state, "Emulator exited abnormally");
	}

	// Always call this in a real fronend
	//finish_frontend(state);
	finish_emulator(state);

	return val;
}
