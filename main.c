#include <stdbool.h>	// bool
#include <stdio.h>	// file methods
#include <stdlib.h>	// exit

#include "params.h"	// system_types
#include "print.h"	// fatal, error, debug
#include "memory.h"	/* _offsets */

// rom_read:
bool read_rom_data(FILE *rom);

// ctl_unit:
void init_ctl(char);
bool execute(void);


int main(int argc, char *argv[])
{
	FILE *rom;

	printf("Super Game Herm!\n");
	printf("Beta version!\n\n");

	if(argc < 2)
	{
		fatal("You must specify a ROM file... -.-");
	}

	rom = fopen(argv[1], "rb");
	if(rom == NULL)
	{
		perror("open rom");
		fatal("Can't open ROM file %s", argv[1]);
	}

	if(!read_rom_data(rom))
	{
		fatal("can't read ROM data (ROM is corrupt)?");
	}

	init_ctl(SYSTEM_SGB);
	while(execute());

	return EXIT_SUCCESS;
}
