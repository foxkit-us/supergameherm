#include "config.h"	// macros, bool

#include <stdio.h>	// file methods
#include <stdlib.h>	// malloc
#include <string.h>	// memcmp, strerror
#include <errno.h>	// errno
#include <time.h>	// time

#include "sgherm.h"	// emu_state
#include "print.h"	// fatal, error, debug
#include "rom.h"	// constants, cart_header, etc.
#include "util.h"	// likely/unlikely
#include "memmap.h"	// memmap_*
#include "platform/swap.h"	// letoh32, htole32


static const unsigned char graphic_expected[] =
{
	0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83,
	0x00, 0x0C, 0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
	0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99, 0xBB, 0xBB, 0x67, 0x63,
	0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E,
};

const char *friendly_cart_names[0x20] =
{
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

bool read_bootrom_data(emu_state *restrict state, const char *bootrom_path)
{
	int i;
	size_t read_size;
	FILE *bootrom;

	state->bootrom_data = NULL;

	if((i = get_file_size(bootrom_path)) < 0)
	{
		error(state, "Could not get size of bootrom: %s", strerror(errno));
		return false;
	}

	if((read_size = i) > 0x10000)
	{
		// Out of bounds
		error(state, "Bootrom is too large");
		return false;
	}

	if(!(bootrom = fopen(bootrom_path, "r")))
	{
		error(state, "Could not open bootrom: %s", strerror(errno));
		return false;
	}

	if(!(state->bootrom_data = malloc(read_size)))
	{
		error(state, "Out of memory");
		fclose(bootrom);
		return false;
	}

	if(fread(state->bootrom_data, read_size, 1, bootrom) < 1)
	{
		error(state, "Couldn't read bootrom: %s", strerror(errno));
		fclose(bootrom);
		free(state->bootrom_data);
		return false;
	}

	state->bootrom_size = read_size;

	fclose(bootrom);

	debug(state, "Read boot ROM (size %ld)", read_size);

	return true;
}

bool read_rom_data(emu_state *restrict state, const char *restrict rom_path,
	cart_header *restrict *restrict header)
{
	int i;
	size_t read_size, cart_size;
	size_t result;
	int8_t checksum = 0;
	char title[20] = "\0", publisher[5] = "\0"; // Max sizes
	const cart_offsets begin = OFF_GRAPHIC_BEGIN;
	bool err = true;
	FILE *rom = NULL;

	// Initalise
	*header = NULL;

	// Get ROM size
	if((i = get_file_size(rom_path)) < 0)
	{
		fatal(state, "Could not get size of ROM: %s", strerror(errno));
		goto close_rom;
	}

	state->cart_size = cart_size = i;

	if(unlikely(cart_size < 0x8000))
	{
		fatal(state, "ROM is too small");
		goto close_rom;
	}

	if((rom = fopen(rom_path, "rb")) == NULL)
	{
		fatal(state, "Could not open ROM: %s", strerror(errno));
		goto close_rom;
	}

	state->cart_size = cart_size;

	if(unlikely((state->cart_data = (uint8_t *)malloc(cart_size)) == NULL))
	{
		fatal(state, "Could not allocate RAM for ROM");
		goto close_rom;
	}

	result = fread(state->cart_data, 1, cart_size, rom);

	if(unlikely(result != cart_size))
	{
		fatal(state, "Could not read ROM: %s", strerror(errno));
		goto close_rom;
	}

	*header = (cart_header *)(state->cart_data + (size_t)begin);
	if(unlikely(memcmp((*header)->graphic, graphic_expected,
			   sizeof(graphic_expected)) != 0))
	{
#ifdef NDEBUG
		error(state, "invalid nintendo graphic (don't care)");
#else
		fatal(state, "invalid nintendo graphic!");
		goto close_rom;
#endif //NDEBUG
	}
	else
	{
		debug(state, "valid nintendo graphic found");
	}

	if ((*header)->cgb_title.compat & 0x80)
	{
		// Game boy color[sic]
		strncpy(title, (*header)->cgb_title.title,
			sizeof((*header)->cgb_title.title));
		strncpy(publisher, (*header)->cgb_title.publisher,
			sizeof((*header)->cgb_title.publisher));

		state->system = SYSTEM_CGB;

		debug(state, "cart type is CGB");
	}
	else if ((*header)->sgb_title.sgb & 0x03)
	{
		// Super game boy
		strncpy(title, (*header)->sgb_title.title,
			sizeof((*header)->sgb_title.title));

		state->system = SYSTEM_SGB;

		debug(state, "cart type is SGB");
	}
	else
	{
		// Really old cart predating the SGB
		strncpy(title, (*header)->dmg_title, sizeof((*header)->dmg_title));

		state->system = SYSTEM_DMG;

		debug(state, "cart type is DMG");
	}

	debug(state, "loading cart %s", title);
	if(*publisher)
	{
		debug(state, "publisher %s", publisher);
	}

	if(unlikely((*header)->cart_type > 0x20))
	{
		debug(state, "type is unknown (%X)", (*header)->cart_type);
	}
	else
	{
		debug(state, "type: %s", friendly_cart_names[(*header)->cart_type]);
	}

	debug(state, "Header size is %d\n", (*header)->rom_size);
	read_size = (uint64_t)0x8000 << (*header)->rom_size;

	if(cart_size != read_size)
	{
		fatal(state, "ROM size %ld is not the expected %ld bytes",
		      cart_size, read_size);
		goto close_rom;
	}

	for(i = 0x134; i <= 0x14d; ++i)
	{
		checksum += state->cart_data[i] + 1;
	}

	if(checksum != 1)
	{
		warning(state, "invalid header checksum! expected %02X, got %02X (a real DMG doesn't care)",
			(1 + state->cart_data[0x14D] - checksum) & 0xFF,
			state->cart_data[0x14D]);
	}
	else
	{
		debug(state, "Valid header checksum found");
	}

	// FIXME For now we're targeting DMG, not CGB.
	state->system = SYSTEM_DMG;

	// Select the correct MBC
	if(!mbc_select(state))
	{
		goto close_rom;
	}

	err = false;
close_rom:
	if(err)
	{
		free(state->cart_data);
	}

	if(rom)
	{
		fclose(rom);
	}

	return !err;
}

/*!
 * Save RTC data to file
 * @param state state structure
 * @warning you must call read_rom_data first!
 */
void rtc_load(emu_state *state)
{
	const uint32_t ram_total = state->mbc.ram_total;
	uint32_t data[12];

	if(ram_total <= 0)
	{
		info(state, "Not loading save state, cart is ROM only");
		return;
	}

	// XXX MBC3 has RTC bits but this loading stuff doesn't belong here
	switch(state->mbc.cart)
	{
	case CART_MBC3_TIMER_BATT:
	case CART_MBC3_TIMER_RAM_BATT:
	case CART_MBC3:
	case CART_MBC3_RAM:
	case CART_MBC3_RAM_BATT:
		memcpy(data, state->mbc.cart_ram + ram_total, sizeof(data));

		// Standard VBA format
		state->mbc.mbc3.rtc[0].seconds   = le32toh(data[0]) & 0xFF;
		state->mbc.mbc3.rtc[0].minutes   = le32toh(data[1]) & 0xFF;
		state->mbc.mbc3.rtc[0].hours     = le32toh(data[2]) & 0xFF;
		state->mbc.mbc3.rtc[0].days      = le32toh(data[3]) & 0xFF;
		state->mbc.mbc3.rtc[0].day_carry = le32toh(data[4]) & 0x01;

		state->mbc.mbc3.rtc[1].seconds   = le32toh(data[5]) & 0xFF;
		state->mbc.mbc3.rtc[1].minutes   = le32toh(data[6]) & 0xFF;
		state->mbc.mbc3.rtc[1].hours     = le32toh(data[7]) & 0xFF;
		state->mbc.mbc3.rtc[1].days      = le32toh(data[8]) & 0xFF;
		state->mbc.mbc3.rtc[1].day_carry = le32toh(data[9]) & 0x01;

		state->mbc.mbc3.unix_time_last   = le32toh(data[10]);
		state->mbc.mbc3.unix_time_last  |= (uint64_t)(le32toh(data[11])) << 32;

		// TODO unix time offset stuff
		info(state, "RTC data loaded");
	default:
		// Call it done
		return;
	}
}

/*!
 * Save RTC data to file
 * @param state state structure
 */
void rtc_save(emu_state *state)
{
	const uint32_t ram_total = state->mbc.ram_total;
	uint32_t data[12];

	if(ram_total <= 0)
	{
		info(state, "Not writing save state, cart is ROM only");
		return;
	}

	// XXX MBC3 has RTC bits but this loading stuff doesn't belong here
	switch(state->mbc.cart)
	{
	case CART_MBC3_TIMER_BATT:
	case CART_MBC3_TIMER_RAM_BATT:
	case CART_MBC3:
	case CART_MBC3_RAM:
	case CART_MBC3_RAM_BATT:
		// Standard VBA format
		data[0]  = htole32(state->mbc.mbc3.rtc[0].seconds);
		data[1]  = htole32(state->mbc.mbc3.rtc[0].minutes);
		data[2]  = htole32(state->mbc.mbc3.rtc[0].hours);
		data[3]  = htole32(state->mbc.mbc3.rtc[0].days);
		data[4]  = htole32(state->mbc.mbc3.rtc[0].day_carry);

		data[5]  = htole32(state->mbc.mbc3.rtc[1].seconds);
		data[6]  = htole32(state->mbc.mbc3.rtc[1].minutes);
		data[7]  = htole32(state->mbc.mbc3.rtc[1].hours);
		data[8]  = htole32(state->mbc.mbc3.rtc[1].days);
		data[9]  = htole32(state->mbc.mbc3.rtc[1].day_carry);

		data[10] = htole32(state->mbc.mbc3.unix_time_last & 0xFFFFFFFF);
		data[11] = htole32(state->mbc.mbc3.unix_time_last >> 32);

		memcpy(state->mbc.cart_ram + ram_total, data, sizeof(data));

		// Need writeback
		state->mbc.dirty = true;

		//info(state, "RTC data saved");
	default:
		// Call it done
		return;
	}
}
