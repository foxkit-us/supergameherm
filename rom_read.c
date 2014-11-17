#include <stdbool.h>	// bool
#include <stdio.h>	// file methods
#include <stdlib.h>	// malloc
#include <string.h>	// memcmp
#include <byteswap.h>	// __bswap_16

#include "print.h"	// fatal, error, debug
#include "memory.h"	// offsets

/*! actual loaded ROM data */
unsigned char *data;
extern unsigned char memory[0x10000];

#define ROM_NINTENDO 0x0104
#define ROM_TITLE 0x0134
#define ROM_TITLE_LEN 16
#define ROM_COLOR 0x0143
#define ROM_LICENSEE 0x0144
#define ROM_SGB 0x0146
#define ROM_CART_TYPE 0x0147
#define ROM_SIZE 0x0148
#define ROM_RAMSIZE 0x0149
#define ROM_DESTINATION 0x014A
#define ROM_OLD_LICENSEE 0x014B
#define ROM_VERSION 0x014C
#define ROM_COMPLEMENT 0x014D
#define ROM_CHECKSUM 0x014E

enum cart_types
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
};

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

bool read_rom_data(FILE *rom)
{
	const char graphic_expected[48] = {
		0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D,
		0x00, 0x0B, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00,
		0x0D, 0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
		0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99, 0xBB,
		0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC,
		0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E };
	long size_in_bytes, actual_size;
	cartridge_header header;
	const enum offsets begin = graphic_begin;

	if(fseek(rom, begin, SEEK_SET))
	{
		perror("seeking");
		return false;
	}

	if (fread(&header, sizeof(cartridge_header), 1, rom) != 1)
	{
		perror("Error reading ROM header");
		return false;
	}

	if(memcmp(header.graphic, graphic_expected, sizeof(graphic_expected)) != 0)
	{
#ifdef NDEBUG
		error("invalid nintendo graphic (don't care)");
#else
		fatal("invalid nintendo graphic!");
#endif
	} else {
		debug("valid nintendo graphic found");
	}

	if (header.gbc_title.compat & 0x80)
	{
		/* Game boy color[sic] */
		char title[sizeof(header.gbc_title.title) + 1];
		char publisher[sizeof(header.gbc_title.publisher) + 1];
		memcpy(title, header.gbc_title.title, sizeof(header.gbc_title.title));
		memcpy(publisher, header.gbc_title.publisher, sizeof(header.gbc_title.publisher));

		title[sizeof(header.gbc_title.title)] = '\0';
		publisher[sizeof(header.gbc_title.publisher)] = '\0';

		debug("loading cartridge %s", title);
		debug("publisher %s", publisher);
	}
	else if (header.sgb_title.sgb & 0x03)
	{
		/* Super game boy */
		char title[sizeof(header.sgb_title.title) + 1];
		memcpy(title, header.sgb_title.title, sizeof(header.sgb_title.title));
		title[sizeof(header.sgb_title.title)] = '\0';
		debug("loading cartridge %s", title);
	}
	else
		/* Really old cart predating the SGB */
		debug("loading cartridge %s", header.old_title);

	debug("Header size is %d\n", header.rom_size);
	size_in_bytes = (1 << ((header.rom_size & 0x7) + 1 /* implicit bank */
				+ 14 /* 16384 bytes per bank */));
	if(header.rom_size & 0x50) size_in_bytes += 1048576;

	if(fseek(rom, 0, SEEK_END))
	{
		perror("seeking");
		return false;
	}

	actual_size = ftell(rom);

	if(actual_size != size_in_bytes)
	{
		fatal("ROM size %ld is not the expected %ld bytes",
		      actual_size, size_in_bytes);
	}

#ifdef BIG_ENDIAN
	header.cartridge_checksum = __bswap_16(header.cartridge_checksum);
#endif

	data = malloc(size_in_bytes);
	if(data == NULL)
	{
		fatal("cannot allocate data segment of %ld bytes",
		      size_in_bytes);
	}

	/* be kind, */ rewind(rom);

	if(fread(data, 1, size_in_bytes, rom) != size_in_bytes)
	{
		perror("reading ROM data");
		return false;
	}

	if(fclose(rom) == EOF)
	{
		error("couldn't close ROM file (libc bug?)");
	}

	memcpy(memory, data, 0x7fff);

	return true;
}
