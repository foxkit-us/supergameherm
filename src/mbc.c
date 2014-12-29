#include "config.h"	// bool
#include "sgherm.h"	// emu_state
#include "memory.h"	// constants
#include "print.h"	// warning/debug
#include "mmap.h"	// memmap_*
#include "util.h"	// unix_time_delta

#include <string.h>	// memset
#include <stdlib.h>	// calloc, free
#include <assert.h>	// assert
#include <time.h>	// time


//! read from the switchable ROM bank space
static inline uint8_t rom_bank_read(emu_state *restrict state, uint16_t location)
{
	uint32_t addr = (state->mbc.rom_bank - 1) * 0x4000;
	if(unlikely(addr > state->cart_size))
	{
		// Out of bounds
		warning(state, "Read out-of-bounds in ROM bank %02X (location %04X)",
			state->mbc.rom_bank, location);
		return 0xFF;
	}

	return state->cart_data[addr + location];
}

//! read from the switchable RAM bank space
static inline uint8_t ram_bank_read(emu_state *restrict state, uint16_t location)
{
	size_t pos = (state->mbc.ram_bank * state->mbc.ram_bank_size) +
		(location - 0xA000);
	uint8_t value;

	if(unlikely(pos > state->mbc.ram_total))
	{
		warning(state, "Attempt to read from nonexistent cart RAM at %d!",
			location);
		return 0xFF;
	}

	value = state->mbc.cart_ram[pos];
	if(state->mbc.use_4bit)
	{
		value |= 0xF0;
	}

	return value;
}

//! write to the switchable RAM bank space
static inline void ram_bank_write(emu_state *restrict state, uint16_t location, uint8_t value)
{
	size_t pos = (state->mbc.ram_bank * state->mbc.ram_bank_size) +
		(location - 0xA000);

	if(unlikely(pos > state->mbc.ram_total))
	{
		warning(state, "Attempt to write to nonexistent cart RAM at %d!",
			location);
		return;
	}

	if(state->mbc.use_4bit)
	{
		value |= 0xF0;
	}
	state->mbc.cart_ram[pos] = value;

	state->mbc.dirty = true;

	// Write back to storage
	memmap_sync(state, state->mbc.cart_ram, &(state->mbc.cart_mm_data));
}

// MBC-less operation

static inline bool nombc_init(emu_state *restrict state UNUSED)
{
	int s = state->cart_data[OFF_RAM_SIZE];

	state->mbc.use_4bit = false;

	// Most MBC-less carts don't have RAM... but some rare ones apparently do.
	if(s)
	{
		// Get total memory size
		s = 0x400 << ((s << 1) - 1);

		// I don't know the bank size, assuming 8k banks (or bank size)
		state->mbc.ram_bank_size = (s >= 8192) ? 8192 : s;
		state->mbc.ram_bank_count = s / state->mbc.ram_bank_size;
		state->mbc.ram_total = s;
		state->mbc.cart_ram = memmap_open(state, state->save_path, s,
			&(state->mbc.cart_mm_data));
		if(!(state->mbc.cart_ram))
		{
			return false;
		}
	}
	else
	{
		state->mbc.ram_bank_size = state->mbc.ram_bank_count = 0;
		state->mbc.cart_ram = NULL;
	}

	return true;
}

static inline uint8_t nombc_read(emu_state *restrict state, uint16_t location)
{
	switch(location >> 12)
	{
	case 0x0:
	case 0x1:
	case 0x2:
	case 0x3:
	case 0x4:
	case 0x5:
	case 0x6:
	case 0x7:
		return state->cart_data[location];
	default:
		warning(state, "Unimplemented read for ROM at address %04X", location);
		return 0xFF;
	}
}

static inline void nombc_write(emu_state *restrict state, uint16_t location, uint8_t value UNUSED)
{
	warning(state, "Unimplemented write for ROM at address %04X", location);
}

static inline void nombc_finish(emu_state *restrict state)
{
	memmap_close(state, state->mbc.cart_ram, &(state->mbc.cart_mm_data));
}

const mbc_func nombc_func =
{
	&nombc_init,
	&nombc_read,
	&nombc_write,
	&nombc_finish,
};

// MBC1 functions

static inline bool mbc1_init(emu_state *restrict state)
{
	int s = state->cart_data[OFF_RAM_SIZE];

	state->mbc.rom_bank = 1;
	state->mbc.use_4bit = false;
	state->mbc.mbc_common.rom_select = false;
	state->mbc.mbc_common.ram_enable = 1;

	// TODO load state with batt
	if(s)
	{
		// Get total memory size
		s = 0x400 << ((s << 1) - 1);

		// MBC1 uses 8k banks (like most carts)
		state->mbc.ram_bank_size = (s >= 8192) ? 8192 : s;
		state->mbc.ram_bank_count = s / state->mbc.ram_bank_size;
		state->mbc.ram_total = s;
		state->mbc.cart_ram = memmap_open(state, state->save_path, s,
			&(state->mbc.cart_mm_data));
		if(!(state->mbc.cart_ram))
		{
			return false;
		}
	}
	else
	{
		state->mbc.ram_bank_size = state->mbc.ram_bank_count =
			state->mbc.ram_total = 0;
		state->mbc.cart_ram = NULL;
	}

	return true;
}

static inline uint8_t mbc1_read(emu_state *restrict state, uint16_t location)
{
	switch(location >> 12)
	{
	case 0x0:
	case 0x1:
	case 0x2:
	case 0x3:
		return state->cart_data[location];
	case 0x4:
	case 0x5:
	case 0x6:
	case 0x7:
		return rom_bank_read(state, location);
	case 0xA:
	case 0xB:
		if((state->mbc.mbc_common.ram_enable & 0xA) == 0xA)
		{
			// switchable RAM bank - 0xA000..0xBFFF
			return ram_bank_read(state, location);
		}

		return 0xFF;
	default:
		warning(state, "Unimplemented read for MBC1 at address %04X", location);
		return 0xFF;
	}
}

static inline void mbc1_write(emu_state *restrict state, uint16_t location, uint8_t value)
{
	switch(location >> 12)
	{
	case 0x0:
	case 0x1:
		state->mbc.mbc_common.ram_enable = value;
		break;
	case 0x2:
	case 0x3:
		// Bank switch
		if(value == 0)
		{
			// 0 gets remapped to 1
			value = 1;
		}

		state->mbc.rom_bank = (value & 0x1F) |
			(state->mbc.rom_bank & 0x60);
		break;
	case 0x4:
	case 0x5:
		if(state->mbc.mbc_common.rom_select)
		{
			// RAM banking mode
			state->mbc.ram_bank = value;
		}
		else
		{
			// ROM banking mode
			state->mbc.rom_bank = (state->mbc.rom_bank & 0x1F) |
				((value & 0x3) << 5);
		}

		break;
	case 0x6:
	case 0x7:
		// TODO enforce mode 0/1 limitations
		state->mbc.mbc_common.rom_select = value;
		break;
	case 0xA:
	case 0xB:
		// switchable RAM bank - 0xA000..0xBFFF
		if((state->mbc.mbc_common.ram_enable & 0xA) == 0xA)
		{
			ram_bank_write(state, location, value);
		}
		break;
	default:
		warning(state, "Unimplemented write for MBC1 at address %04X", location);
		break;
	}
}

static inline void mbc1_finish(emu_state *restrict state)
{
	memmap_close(state, state->mbc.cart_ram, &(state->mbc.cart_mm_data));
}

const mbc_func mbc1_func =
{
	&mbc1_init,
	&mbc1_read,
	&mbc1_write,
	&mbc1_finish,
};

// MBC2

static inline bool mbc2_init(emu_state *restrict state)
{
	state->mbc.rom_bank = 1;
	state->mbc.mbc_common.rom_select = false;
	state->mbc.mbc_common.ram_enable = 0;

	// MBC2 does *not* use 8K banks, it only has 512 4-bit values.
	state->mbc.ram_bank_size = 512;
	state->mbc.ram_bank_count = 1;
	state->mbc.ram_total = 512;
	state->mbc.use_4bit = true;
	state->mbc.cart_ram = memmap_open(state, state->save_path,
		state->mbc.ram_bank_size, &(state->mbc.cart_mm_data));
	if(!(state->mbc.cart_ram))
	{
		return false;
	}

	return true;
}

static inline uint8_t mbc2_read(emu_state *restrict state, uint16_t location)
{
	switch(location >> 12)
	{
	case 0x0:
	case 0x1:
	case 0x2:
	case 0x3:
		return state->cart_data[location];
	case 0x4:
	case 0x5:
	case 0x6:
	case 0x7:
		// switchable bank - 0x4000..0x7FFF (most MBC's)
		return rom_bank_read(state, location);
	case 0xA:
	case 0xB:
		if(location > 0xA1FF || !(state->mbc.mbc_common.ram_enable))
		{
			// Limited on MBC2
			return 0xFF;
		}

		return ram_bank_read(state, location);
	default:
		warning(state, "Unimplemented read for MBC2 at address %04X", location);
		return 0xFF;
	}
}

static inline void mbc2_write(emu_state *restrict state, uint16_t location, uint8_t value)
{
	switch(location >> 12)
	{
	case 0x0:
	case 0x1:
		if(location & 0x100)
		{
			break;
		}

		state->mbc.mbc_common.ram_enable = value;
		break;
	case 0x2:
	case 0x3:
		if(location & 0x100)
		{
			state->mbc.rom_bank = value & 0x1F;
		}
		break;
	case 0xA:
		if(location > 0xA1FF || !(state->mbc.mbc_common.ram_enable))
		{
			// Limited on MBC2
			break;
		}

		ram_bank_write(state, location, value);
		break;
	default:
		warning(state, "Unimplemented write for MBC2 at address %04X", location);
		return;
	}
}

static inline void mbc2_finish(emu_state *restrict state)
{
	memmap_close(state, state->mbc.cart_ram, &(state->mbc.cart_mm_data));
}

const mbc_func mbc2_func =
{
	&mbc2_init,
	&mbc2_read,
	&mbc2_write,
	&mbc2_finish,
};

// MBC3

// XXX exported for rom.c stuff
void adjust_mbc3_time(emu_state *restrict state)
{
	time_delta td;
	uint64_t t = time(NULL);

	if(state->mbc.mbc3.rtc[0].halt)
	{
		// Timer halted, no adjustment required
		return;
	}

	if(t == state->mbc.mbc3.unix_time_last)
	{
		// No need to redo this expensive calculation
		return;
	}

	unix_time_delta(t, state->mbc.mbc3.unix_time_last, &td);

	// Set
	state->mbc.mbc3.unix_time_last = t;

	state->mbc.mbc3.rtc[0].seconds += td.seconds;
	if(state->mbc.mbc3.rtc[0].seconds >= 60)
	{
		state->mbc.mbc3.rtc[0].minutes += state->mbc.mbc3.rtc[0].seconds / 60;
		state->mbc.mbc3.rtc[0].seconds %= 60;
	}

	state->mbc.mbc3.rtc[0].minutes += td.minutes;
	if(state->mbc.mbc3.rtc[0].minutes >= 60)
	{
		state->mbc.mbc3.rtc[0].hours += state->mbc.mbc3.rtc[0].minutes / 60;
		state->mbc.mbc3.rtc[0].minutes %= 60;
	}

	state->mbc.mbc3.rtc[0].hours += td.hours;
	if(state->mbc.mbc3.rtc[0].hours >= 24)
	{
		state->mbc.mbc3.rtc[0].days += state->mbc.mbc3.rtc[0].hours / 24;
		state->mbc.mbc3.rtc[0].hours %= 24;
	}

	state->mbc.mbc3.rtc[0].days += td.days;
	if(state->mbc.mbc3.rtc[0].days > 512)
	{
		state->mbc.mbc3.rtc[0].day_carry = 1;
		state->mbc.mbc3.rtc[0].days %= 512;
	}

	// Write back
	rtc_save(state);
}

static inline bool mbc3_init(emu_state *restrict state)
{
	int s = state->cart_data[OFF_RAM_SIZE];

	state->mbc.rom_bank = 1;
	state->mbc.use_4bit = false;
	state->mbc.mbc_common.rom_select = false;
	state->mbc.mbc_common.ram_enable = 1;

	// TODO load state with batt
	if(s)
	{
		// Get total memory size
		s = 0x400 << ((s << 1) - 1);

		// Just like MBC1
		state->mbc.ram_bank_size = (s >= 8192) ? 8192 : s;
		state->mbc.ram_bank_count = s / state->mbc.ram_bank_size;
		state->mbc.ram_total = s;

		// Compensate for RTC data (in-file)
		state->mbc.cart_ram = memmap_open(state, state->save_path, s + 48,
			&(state->mbc.cart_mm_data));
		if(!(state->mbc.cart_ram))
		{
			return false;
		}
	}
	else
	{
		state->mbc.ram_bank_size = state->mbc.ram_bank_count =
			state->mbc.ram_total = 0;
		state->mbc.cart_ram = NULL;
	}

	rtc_load(state);
	adjust_mbc3_time(state);

	return true;
}

static inline uint8_t mbc3_read(emu_state *restrict state, uint16_t location)
{
	switch(location >> 12)
	{
	case 0x0:
	case 0x1:
	case 0x2:
	case 0x3:
		return state->cart_data[location];
	case 0x4:
	case 0x5:
	case 0x6:
	case 0x7:
		// switchable bank - 0x4000..0x7FFF (most MBC's)
		return rom_bank_read(state, location);
	case 0xA:
	case 0xB:
	{
		// Select latched or unlatched value
		size_t l_index = state->mbc.mbc3.latched == 1 ? 1 : 0;

		if(!((state->mbc.mbc3.ram_rtc_enable & 0xA) == 0xA))
		{
			return 0xFF;
		}

		if(l_index == 0 && state->mbc.mbc3.rtc_select >= 0x8)
		{
			// Unlatched, sync clock
			adjust_mbc3_time(state);
		}

		// Select RTC or RAM bank
		switch(state->mbc.mbc3.rtc_select)
		{
		case 0x8:
			return state->mbc.mbc3.rtc[l_index].seconds;
		case 0x9:
			return state->mbc.mbc3.rtc[l_index].minutes;
		case 0xA:
			return state->mbc.mbc3.rtc[l_index].hours;
		case 0xB:
			return state->mbc.mbc3.rtc[l_index].days & 0xFF;
		case 0xC:
		{
			uint8_t ret = (state->mbc.mbc3.rtc[l_index].days & 0x100) >> 8;
			ret |= state->mbc.mbc3.rtc[l_index].halt << 6;
			ret |= state->mbc.mbc3.rtc[l_index].day_carry << 7;
			return ret;
		}
		default:
			return ram_bank_read(state, location);
		}

		break;
	}
	default:
		warning(state, "Unimplemented read for MBC3 at address %04X", location);
		return 0xFF;
	}
}

static inline void mbc3_write(emu_state *restrict state, uint16_t location, uint8_t value)
{
	switch(location >> 12)
	{
	case 0x0:
	case 0x1:
		state->mbc.mbc3.ram_rtc_enable = value;
		break;
	case 0x2:
	case 0x3:
		// Bank switch
		state->mbc.rom_bank = value & 0x7F;
		break;
	case 0x4:
	case 0x5:
		state->mbc.mbc3.rtc_select = value;
		if(state->mbc.mbc3.rtc_select < 0x4)
		{
			state->mbc.ram_bank = value;
		}
		break;
	case 0xA:
	case 0xB:
	{
		//Select latched or unlatched value
		size_t l_index = state->mbc.mbc3.latched == 1 ? 1 : 0;

		if(!((state->mbc.mbc3.ram_rtc_enable & 0xA) == 0xA))
		{
			return;
		}

		if(l_index == 0 && state->mbc.mbc3.rtc_select >= 0x8)
		{
			// Unlatched, sync clock
			adjust_mbc3_time(state);
		}


		// Select RTC or RAM bank
		switch(state->mbc.mbc3.rtc_select)
		{
		case 0x8:
			state->mbc.mbc3.rtc[l_index].seconds = value;
			return;
		case 0x9:
			state->mbc.mbc3.rtc[l_index].minutes = value;
			return;
		case 0xA:
			state->mbc.mbc3.rtc[l_index].hours = value;
			return;
		case 0xB:
			state->mbc.mbc3.rtc[l_index].days =
				(state->mbc.mbc3.rtc[l_index].days & 0x100) | value;
			return;
		case 0xC:
		{
			state->mbc.mbc3.rtc[l_index].days =
				(state->mbc.mbc3.rtc[l_index].days & 0xFF) |
				((value & 0x1) << 8);
			state->mbc.mbc3.rtc[l_index].halt = (value & 0x40) >> 6;
			state->mbc.mbc3.rtc[l_index].day_carry = (value & 0x80) >> 7;
			return;
		}
		default:
			// switchable RAM bank - 0xA000..0xBFFF
			ram_bank_write(state, location, value);
			return;
		}
		break;
	}
	case 0x6:
	case 0x7:
		// Latch RTC value
		if(value && state->mbc.mbc3.latched == 0)
		{
			// Do adjustment before latch
			adjust_mbc3_time(state);

			// Copy the present RTC value
			memcpy(&(state->mbc.mbc3.rtc[1]), &(state->mbc.mbc3.rtc[0]),
				sizeof(state->mbc.mbc3.rtc[1]));
		}

		state->mbc.mbc3.latched = value;
		break;
	default:
		warning(state, "Unimplemented write for MBC3 at address %04X", location);
		break;
	}
}

static inline void mbc3_finish(emu_state *restrict state)
{
	rtc_save(state);
	memmap_close(state, state->mbc.cart_ram, &(state->mbc.cart_mm_data));
}

const mbc_func mbc3_func =
{
	&mbc3_init,
	&mbc3_read,
	&mbc3_write,
	&mbc3_finish,
};

// MBC5 (there was no MBC4 according to sources due to Japanese superstitions
// regarding the number 4)

static inline bool mbc5_init(emu_state *restrict state)
{
	// Init is same as MBC1
	if(!mbc1_init(state))
	{
		return false;
	}

	return true;
}

static inline uint8_t mbc5_read(emu_state *restrict state, uint16_t location)
{
	switch(location >> 12)
	{
	case 0x0:
	case 0x1:
	case 0x2:
	case 0x3:
		return state->cart_data[location];
	case 0x4:
	case 0x5:
	case 0x6:
	case 0x7:
		// switchable bank - 0x4000..0x7FFF (most MBC's)
		return rom_bank_read(state, location);
	case 0xA:
	case 0xB:
		if((state->mbc.mbc_common.ram_enable & 0xA) == 0xA)
		{
			// switchable RAM bank - 0xA000..0xBFFF
			return ram_bank_read(state, location);
		}
		return 0xFF;
	default:
		warning(state, "Unimplemented read for MBC5 at address %04X", location);
		return 0xFF;
	}
}

static inline void mbc5_write(emu_state *restrict state, uint16_t location, uint8_t value)
{
	switch(location >> 12)
	{
	case 0x0:
	case 0x1:
		state->mbc.mbc_common.ram_enable = value;
		break;
	case 0x2:
		state->mbc.rom_bank = (state->mbc.rom_bank & 0x100) | value;
		break;
	case 0x3:
		state->mbc.rom_bank |= (value & 0x1) << 8;
		break;
	case 0x4:
	case 0x5:
		// RAM banking mode
		state->mbc.ram_bank = value;
		break;
	case 0xA:
	case 0xB:
		if((state->mbc.mbc_common.ram_enable & 0xA) == 0xA)
		{
			// switchable RAM bank - 0xA000..0xBFFF
			ram_bank_write(state, location, value);
		}
		break;
	default:
		warning(state, "Unimplemented write for MBC5 at address %04X", location);
		break;
	}
}

static inline void mbc5_finish(emu_state *restrict state)
{
	memmap_close(state, state->mbc.cart_ram, &(state->mbc.cart_mm_data));
}

const mbc_func mbc5_func =
{
	&mbc5_init,
	&mbc5_read,
	&mbc5_write,
	&mbc5_finish,
};

// HuC1

static inline bool huc1_init(emu_state *restrict state)
{
	// Same as MBC1 with an IR controller I know nothing about
	return mbc1_init(state);
}

static inline uint8_t huc1_read(emu_state *restrict state, uint16_t location)
{
	// Same as MBC1 with an IR controller I know nothing about
	return mbc1_read(state, location);
}

static inline void huc1_write(emu_state *restrict state, uint16_t location, uint8_t value)
{
	// Same as MBC1 with an IR controller I know nothing about
	mbc1_write(state, location, value);
}

static inline void huc1_finish(emu_state *restrict state)
{
	// I think you get the idea.
	mbc1_finish(state);
}

const mbc_func huc1_func =
{
	&huc1_init,
	&huc1_read,
	&huc1_write,
	&huc1_finish,
};

// HuC3

static inline bool huc3_init(emu_state *restrict state)
{
	// TODO
	fatal(state, "Unimplemented cartridge variant HUC3");
	return false;
}

static inline uint8_t huc3_read(emu_state *restrict state UNUSED, uint16_t location UNUSED)
{
	return 0xFF;
}

static inline void huc3_write(emu_state *restrict state UNUSED, uint16_t location UNUSED, uint8_t value UNUSED)
{
}

static inline void huc3_finish(emu_state *restrict state UNUSED)
{
}

const mbc_func huc3_func = {
	&huc3_init,
	&huc3_read,
	&huc3_write,
	&huc3_finish,
};

//! Select an MBC for the cartridge
bool mbc_select(emu_state *restrict state)
{
	cart_types mbc_type = state->cart_data[OFF_CART_TYPE];

	state->mbc.cart = mbc_type;

	// TODO - determine HuC1/Huc3 ROM types and stuff
	switch(mbc_type)
	{
	case CART_ROM_ONLY:
	case CART_RAM:
	case CART_RAM_BATT:
		debug(state, "ROM only cartridge");
		state->mbc.func = &nombc_func;
		break;
	case CART_MBC1:
	case CART_MBC1_RAM:
	case CART_MBC1_RAM_BATT:
		debug(state, "MBC1 cartridge");
		state->mbc.func = &mbc1_func;
		break;
	case CART_MBC2:
	case CART_MBC2_BATT:
		debug(state, "MBC2 cartridge");
		state->mbc.func = &mbc2_func;
		break;
	case CART_MBC3_TIMER_BATT:
	case CART_MBC3_TIMER_RAM_BATT:
	case CART_MBC3:
	case CART_MBC3_RAM:
	case CART_MBC3_RAM_BATT:
		debug(state, "MBC3 cartridge");
		state->mbc.func = &mbc3_func;
		break;
	case CART_MBC5:
	case CART_MBC5_RAM:
	case CART_MBC5_RAM_BATT:
	case CART_MBC5_RUMBLE:
	case CART_MBC5_RUMBLE_SRAM:
	case CART_MBC5_RUMBLE_SRAM_BATT:
		debug(state, "MBC5 cartridge");
		state->mbc.func = &mbc5_func;
		break;
	default:
		fatal(state, "Unknown MBC type %d\n", mbc_type);
		return false;
	}

	debug(state, "Initalising MBC...");
	return MBC_INIT(state);
}
