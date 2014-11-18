#include <stdbool.h>	// bool
#include <stdio.h>	// file methods
#include <stdlib.h>	// malloc
#include <string.h>	// memcmp
#include <byteswap.h>	// __bswap_16

#include "print.h"	// fatal, error, debug
#include "memory.h"	// offsets, emulator_state

static const unsigned char graphic_expected[] = {
	0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83,
	0x00, 0x0C, 0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
	0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99, 0xBB, 0xBB, 0x67, 0x63,
	0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E,
};

typedef enum
{
	CART_ROM_ONLY = 0x00,
	CART_MBC1 = 0x01,
	CART_MBC1_RAM = 0x02,
	CART_MBC1_RAM_BATT = 0x03,
	CART_MBC2 = 0x05,
	CART_MBC2_BATT = 0x06,
	CART_RAM = 0x08,
	CART_RAM_BATT = 0x09,
	CART_MMM01 = 0x0B,
	CART_MMM01_SRAM = 0x0C,
	CART_MMM01_SRAM_BATT = 0x0D,
	CART_MBC3_TIMER_BATT = 0x0F,
	CART_MBC3_TIMER_RAM_BATT = 0x10,
	CART_MBC3 = 0x11,
	CART_MBC3_RAM = 0x12,
	CART_MBC3_RAM_BATT = 0x13,
	CART_MBC5 = 0x19,
	CART_MBC5_RAM = 0x1A,
	CART_MBC5_RAM_BATT = 0x1B,
	CART_MBC5_RUMBLE = 0x1C,
	CART_MBC5_RUMBLE_SRAM = 0x1D,
	CART_MBC5_RUMBLE_SRAM_BATT = 0x1E,
	CART_CAMERA = 0x1F
} cart_types;

const char *friendly_cart_names[0x20] = {
	"ROM only", "MBC1", "MBC1 with RAM", "MBC1 with RAM (Battery)",
	"unused", "MBC2", "MBC2 (Battery)", "unused", "RAM",
	"RAM (Battery)", "unused", "MMM01", "MMM01 with SRAM",
	"MMM01 with SRAM (Battery)", "unused",
	"MBC3 with Timer (Battery)",
	"MBC3 with Timer and RAM (Battery)", "MBC3", "MBC3 with RAM",
	"MBC3 with RAM (Battery)", "unused", "unused", "unused", "unused",
	"unused", "MBC5", "MBC5 with RAM", "MBC5 with RAM (Battery)",
	"MBC5 Rumble Cart", "MBC5 Rumble Cart with SRAM",
	"MBC5 Rumble Cart with SRAM (Battery)", "GB Pocket Camera"
};

bool read_rom_data(emulator_state *state, FILE *rom, cart_header **header,
		system_types *system)
{
	long size_in_bytes, actual_size;
	int8_t checksum = 0;
	char title[19] = "\0", publisher[5] = "\0"; // Max sizes
	const offsets begin = OFF_GRAPHIC_BEGIN;
	bool err = true;

	/* Initalise */
	*header = NULL;

	/* Get the full ROM size */
	if(unlikely(fseek(rom, 0, SEEK_END)))
	{
		perror("seeking");
		goto close_rom;
	}

	if(unlikely((actual_size = ftell(rom)) < 0x8000))
	{
		error("ROM is too small");
		goto close_rom;
	}

	if(unlikely((state->cart_data = malloc(actual_size)) == NULL))
	{
		error("Could not allocate RAM for ROM");
		goto close_rom;
	}

	if(unlikely(fseek(rom, 0, SEEK_SET)))
	{
		perror("seeking");
		goto close_rom;
	}

	if(unlikely(fread(state->cart_data, actual_size, 1, rom) != 1))
	{
		perror("Could not read ROM");
		goto close_rom;
	}

	*header = (cart_header *)(state->cart_data + (size_t)begin);
	if(unlikely(memcmp((*header)->graphic, graphic_expected,
				sizeof(graphic_expected)) != 0))
	{
#ifdef NDEBUG
		error("invalid nintendo graphic (don't care)");
#else
		fatal("invalid nintendo graphic!");
		goto close_rom;
#endif
	}
	else
	{
		debug("valid nintendo graphic found");
	}

	if ((*header)->gbc_title.compat & 0x80)
	{
		/* Game boy color[sic] */
		strncpy(title, (*header)->gbc_title.title,
				sizeof((*header)->gbc_title.title));
		strncpy(publisher, (*header)->gbc_title.publisher,
						sizeof((*header)->gbc_title.publisher));

		*system = SYSTEM_GBC;
	}
	else if ((*header)->sgb_title.sgb & 0x03)
	{
		/* Super game boy */
		strncpy(title, (*header)->sgb_title.title,
			sizeof((*header)->sgb_title.title));

		*system = SYSTEM_SGB;
	}
	else
	{
		/* Really old cart predating the SGB */
		strncpy(title, (*header)->gb_title, sizeof((*header)->gb_title));

		*system = SYSTEM_GB;
	}

	switch (*system)
	{
	case SYSTEM_GBC:
		debug("cart type is GBC");
		break;
	case SYSTEM_SGB:
		debug("cart type is SGB");
		break;
	case SYSTEM_GB:
	default:
		debug("cart type is GB");
		break;
	}

	debug("loading cart %s", title);
	if(*publisher)
		debug("publisher %s", publisher);

	debug("Header size is %d\n", (*header)->rom_size);
	size_in_bytes = 0x8000 << (*header)->rom_size;

	if(actual_size != size_in_bytes)
	{
		fatal("ROM size %ld is not the expected %ld bytes",
		      actual_size, size_in_bytes);
		goto close_rom;
	}

	for(size_t i = 0x134; i <= 0x14d; ++i)
		checksum += state->cart_data[i] + 1;

	printf("%d\n", checksum);

	if(checksum != 1)
	{
#ifdef NDEBUG
		error("invalid header checksum (don't care)");
#else
		fatal("invalid header checksum!");
		goto close_rom;
#endif
	}
	else
	{
		debug("Valid header checksum found");
	}

	err = false;
close_rom:
	if(unlikely(err))
		free(state->cart_data);
	else
		memcpy(state->memory, state->cart_data, 0x7fff);

	return (!err);
}
