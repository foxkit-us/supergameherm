#include "config.h"	// macros, uint[XX]_t
#include <assert.h>	// assert
#include <string.h>	// memmove

#include "sgherm.h"	// emu_state
#include "ctl_unit.h"	// int_flag_*
#include "input.h"	// joypad_*
#include "lcdc.h"	// lcdc_read
#include "memory.h"	// Constants and what have you
#include "print.h"	// fatal
#include "rom_read.h"	// OFF_CART_TYPE
#include "serio.h"	// serial_*
#include "sound.h"	// sound_*
#include "timer.h"	// timer_*
#include "util.h"	// likely/unlikely


/***********************************************************************
 * readers
 ***********************************************************************/
typedef uint8_t (*mem_read_fn)(emu_state *restrict, uint16_t);

/*!
 * @brief hardware read catch-all for no hardware present at a port
 * @result emulation stopped because some doofus read from a non-existant
 * 	   device.
 */
uint8_t no_hardware(emu_state *restrict state UNUSED, uint16_t location)
{
	warning(state, "no device present at %04X (emulator bug? incompatible GB?)", location);
	warning(state, "(a real GB spews 0xFF)");
	return 0xFF;
}

static inline uint8_t vram_bank_switch_read(emu_state *restrict state, uint16_t location UNUSED)
{
	return state->lcdc.vram_bank;
}

/*! a table of hardware register read methods */
static mem_read_fn hw_reg_read[0x80] =
{
	joypad_read, /* 00 - P1 - joypad */
	serial_read, /* 01 - SB - serial data */
	serial_read, /* 02 - SC - serial control */
	no_hardware, /* 03 - NO HARDWARE */
	timer_read,  /* 04 - DIV */
	timer_read,  /* 05 - TIMA - timer step */
	timer_read,  /* 06 - TMA - timer count */
	timer_read,  /* 07 - TAC - timer frequency / enable */

	/* 08..0E - NO HARDWARE */
	no_hardware, no_hardware, no_hardware, no_hardware,
	no_hardware, no_hardware, no_hardware,

	int_flag_read, /* 0F - IF - interrupt status.. also CPU-based */

	/* 10..14 - sound mode 1 */
	sound_read, sound_read, sound_read, sound_read, sound_read,

	no_hardware, /* 15 - NO HARDWARE */

	/* 16..19 - sound mode 2 */
	sound_read, sound_read, sound_read, sound_read,
	/* 1A..1E - sound mode 3 */
	sound_read, sound_read, sound_read, sound_read, sound_read,

	no_hardware, /* 1F - NO HARDWARE */

	/* 20..23 - sound mode 4 */
	sound_read, sound_read, sound_read, sound_read,
	/* 24..26 - sound control */
	sound_read, sound_read, sound_read,

	/* 27..2F - NO HARDWARE */
	no_hardware, no_hardware, no_hardware, no_hardware, no_hardware,
	no_hardware, no_hardware, no_hardware, no_hardware,

	/* 30..3F - WAV RAM */
	sound_read, sound_read, sound_read, sound_read, sound_read, sound_read,
	sound_read, sound_read, sound_read, sound_read, sound_read, sound_read,
	sound_read, sound_read, sound_read, sound_read,

	/* 40..45 - LCD controller */
	lcdc_control_read, lcdc_stat_read, lcdc_scroll_read,
	lcdc_scroll_read, lcdc_ly_read, lcdc_lyc_read,

	no_hardware, /* 46 - DMA - DMA transfer and control */

	/* 47..4B - more graphics stuff */
	lcdc_read, lcdc_read, lcdc_read, lcdc_window_read, lcdc_window_read,

	/* 4C..4E - NO HARDWARE */
	no_hardware, no_hardware, no_hardware,

	/* 4F - switch VRAM bank (GBC only) */
	vram_bank_switch_read,

	/* 50..67 - NO HARDWARE */
	no_hardware, no_hardware, no_hardware, no_hardware, /* 0x53 */
	no_hardware, no_hardware, no_hardware, no_hardware, /* 0x57 */
	no_hardware, no_hardware, no_hardware, no_hardware, /* 0x5B */
	no_hardware, no_hardware, no_hardware, no_hardware, /* 0x5F */
	no_hardware, no_hardware, no_hardware, no_hardware, /* 0x63 */
	no_hardware, no_hardware, no_hardware, no_hardware, /* 0x67 */

	/* 68..6B - GBC palette stuff (nothing on GB) */
	bg_pal_ind_read, bg_pal_data_read, sprite_pal_ind_read, sprite_pal_data_read,

	/* 6C..7F - NO HARDWARE */
	no_hardware, no_hardware, no_hardware, no_hardware, /* 0x6F */
	no_hardware, no_hardware, no_hardware, no_hardware, /* 0x73 */
	no_hardware, no_hardware, no_hardware, no_hardware, /* 0x77 */
	no_hardware, no_hardware, no_hardware, no_hardware, /* 0x7B */
	no_hardware, no_hardware, no_hardware, no_hardware  /* 0x7F */
};

/*! directly read from a location in memory */
static inline uint8_t direct_read(emu_state *restrict state, uint16_t location)
{
	return state->memory[location];
}

/*! read from the switchable ROM bank space */
static inline uint8_t rom_bank_read(emu_state *restrict state UNUSED, uint16_t location)
{
	uint32_t addr = (state->bank - 1) * 0x4000;
	return state->cart_data[addr + location];
}

/*! read from the switchable RAM bank space */
static inline uint8_t ram_bank_read(emu_state *restrict state, uint16_t location)
{
	return state->cart_ram[state->ram_bank][location - 0xA000];
}

/*!
 * @brief	Read a byte (8 bits) out of memory.
 * @param	state		The emulator state to use when reading.
 * @param	location	The location in memory to read.
 * @returns	The value of the location in memory.
 * @result	Emulation will terminate if the memory cannot be read.
 */
uint8_t mem_read8(emu_state *restrict state, uint16_t location)
{
	if(state->dma_membar_wait && location <= 0xFE80 && location >= 0xFFFE)
	{
		// XXX check into it and see how this is done
		fatal(state, "Prohibited read during DMA transfer");
		return 0;
	}

	switch(location >> 12)
	{
	case 0x4:
	case 0x5:
	case 0x6:
	case 0x7:
		// switchable bank - 0x4000..0x7FFF
		return rom_bank_read(state, location);
	case 0x8:
	case 0x9:
		// video memory - 0x8000..0x9FFF
		return lcdc_read(state, location);
	case 0xA:
	case 0xB:
		// switchable RAM bank - 0xA000-0xBFFF
		return ram_bank_read(state, location);
	case 0xE:
	case 0xF:
		switch(location >> 8)
		{
		case 0xFE:
			break;
		case 0xFF:
			if(location < 0xFF80)
			{
				return hw_reg_read[location - 0xFF00](state, location);
			}
			else if(location == 0xFFFF)
			{
				return int_mask_flag_read(state, location);
			}

			break;
		default:
			// who knows? the SHADOW knows! - 0xE000..0xFDFF
			location -= 0x2000;
		}
	default:
		return direct_read(state, location);
	}
}

/*!
 * @brief	Read two bytes (16 bits) out of memory.
 * @param	state		The emulator state to use when reading.
 * @param	location	The location in memory to read.
 * @returns	The value of the two bytes at the location, LSB lower.
 * @result	Emulation will terminate if the memory cannot be read.
 */
uint16_t mem_read16(emu_state *restrict state, uint16_t location)
{
	return (mem_read8(state, location + 1) << 8) | mem_read8(state, location);
}


/***********************************************************************
 * writers
 ***********************************************************************/
typedef void (*mem_write8_fn)(emu_state *restrict , uint16_t, uint8_t);

/*!
 * @brief silly name, decent intent
 * @result emulation terminates because some doofus wrote to a r/o reg
 */
void readonly_reg_write(emu_state *restrict state, uint16_t location, uint8_t data)
{
	warning(state, "[%4X] attempted write of %02X to read-only register %04X (a real GB ignores this)",
	      REG_PC(state), data, location);
}

/*!
 * @brief what happens when you poke around randomly
 * @result emulation terminates because some doofus wrote to a device not there
 */
void doofus_write(emu_state *restrict state, uint16_t location, uint8_t data)
{
	warning(state, "[%4X] attempted doofus write of %02X to non-existant device at %04X (a real GB ignores this)",
	      REG_PC(state), data, location);
}

static inline void dma_write(emu_state *restrict state, uint16_t location UNUSED, uint8_t data)
{
	/* TODO FIXME XXX OMG HAX */
	/* this is 'correct' but horribly inaccurate:
	 * this transfer should take 160 µs (640 clocks), and during the
	 * transfer, the CPU locks all memory reads except FE80-FFFE.
	 *
	 * NOTE
	 * The above may not be true anymore, but I'm not sure I implemented
	 * this correctly. --Elizabeth
	 */
	uint16_t start = data << 8;
	assert(location == 0xFF46);
	memmove(state->memory + 0xFE00, state->memory + start, 160);

	state->dma_membar_wait = 640;
}

static inline void vram_bank_switch_write(emu_state *restrict state, uint16_t location UNUSED, uint8_t data)
{
	state->lcdc.vram_bank = data;
}

static mem_write8_fn hw_reg_write[0x80] =
{
	joypad_write, /* 00 - P1 - joypad */
	serial_write, /* 01 - SB - serial data */
	serial_write, /* 02 - SC - serial control */
	doofus_write, /* 03 - NO HARDWARE */
	timer_write,  /* 04 - DIV - CPU register */
	timer_write,  /* 05 - TIMA - timer step */
	timer_write,  /* 06 - TMA - timer count */
	timer_write,  /* 07 - TAC - timer control */

	/* 08..0E - NO HARDWARE */
	doofus_write, doofus_write, doofus_write, doofus_write,
	doofus_write, doofus_write, doofus_write,

	int_flag_write, /* 0F - IF - interrupt control */

	/* 10..14 - sound bank 1 */
	sound_write, sound_write, sound_write, sound_write,
	sound_write,

	doofus_write, /* 15 - NO HARDWARE */

	/* 16..19 - sound bank 2 */
	sound_write, sound_write, sound_write, sound_write,

	/* 1A..1E - sound bank 3 */
	sound_write, sound_write, sound_write, sound_write,
	sound_write,

	doofus_write, /* 1F - NO HARDWARE */

	/* 20..23 - sound bank 4 */
	sound_write, sound_write, sound_write, sound_write,

	/* 24..26 - sound control */
	sound_write, sound_write, sound_write,

	/* 27..2F - NO HARDWARE */
	doofus_write, doofus_write, doofus_write,
	doofus_write, doofus_write, doofus_write,
	doofus_write, doofus_write, doofus_write,

	/* 30..3F - WAV RAM */
	sound_write, sound_write, sound_write, sound_write,
	sound_write, sound_write, sound_write, sound_write,
	sound_write, sound_write, sound_write, sound_write,
	sound_write, sound_write, sound_write, sound_write,

	/* 40..45 - LCD controller */
	lcdc_control_write, lcdc_stat_write, lcdc_scroll_write,
	lcdc_scroll_write, lcdc_ly_write, lcdc_lyc_write,

	dma_write, /* 46 - DMA */

	/* 47..4B - more video */
	lcdc_write, lcdc_write, lcdc_write, lcdc_window_write, lcdc_window_write,

	/* 4C..4E - NO HARDWARE */
	doofus_write, doofus_write, doofus_write,

	/* 4F - VRAM bank switch */
	vram_bank_switch_write,

	/* 50..67 - NO HARDWARE */
	doofus_write, doofus_write, doofus_write, doofus_write, /* 0x53 */
	doofus_write, doofus_write, doofus_write, doofus_write, /* 0x57 */
	doofus_write, doofus_write, doofus_write, doofus_write, /* 0x5B */
	doofus_write, doofus_write, doofus_write, doofus_write, /* 0x5F */
	doofus_write, doofus_write, doofus_write, doofus_write, /* 0x63 */
	doofus_write, doofus_write, doofus_write, doofus_write, /* 0x67 */

	/* 68..6B - GBC palette stuff (nothing on GB) */
	bg_pal_ind_write, bg_pal_data_write, sprite_pal_ind_write, sprite_pal_data_write,

	/* 6C..7F - NO HARDWARE */
	doofus_write, doofus_write, doofus_write, doofus_write, /* 0x6F */
	doofus_write, doofus_write, doofus_write, doofus_write, /* 0x73 */
	doofus_write, doofus_write, doofus_write, doofus_write, /* 0x77 */
	doofus_write, doofus_write, doofus_write, doofus_write, /* 0x7B */
	doofus_write, doofus_write, doofus_write, doofus_write, /* 0x7F */
};

/*!
 * @brief	Write a byte (8 bits) to memory.
 * @param	state		The emulator state to use while writing.
 * @param	location	The location in memory to write.
 * @param	data		The data to write.
 * @result	The data is written to the specified location.
 * 		Emulation will terminate if the memory cannot be written to.
 */
void mem_write8(emu_state *restrict state, uint16_t location, uint8_t data)
{
	if(state->dma_membar_wait && location <= 0xFE80 && location >= 0xFFFE)
	{
		fatal(state, "Prohibited write during DMA transfer");
		return;
	}

	switch(location >> 12)
	{
	case 0x2:
	case 0x3:
		switch(state->cart_data[OFF_CART_TYPE])
		{
		case CART_ROM_ONLY:
			warning(state, "invalid memory write at %04X (%02X) (a real GB ignores this)",
			      location, data);
		case CART_MBC1:
		case CART_MBC1_RAM:
		case CART_MBC1_RAM_BATT:
			state->bank = data & 0x1F;
			return;
		case CART_MBC3:
		case CART_MBC3_RAM:
		case CART_MBC3_RAM_BATT:
		case CART_MBC3_TIMER_BATT:
		case CART_MBC3_TIMER_RAM_BATT:
			state->bank = data & 0x7F;
			return;
		default:
			fatal(state, "banks for this cart (type %04X [%s]) aren't done yet sorry :(",
					state->cart_data[OFF_CART_TYPE],
					friendly_cart_names[location >> 12]);
		}
	case 0x4:
	case 0x5:
		switch(state->cart_data[OFF_CART_TYPE])
		{
		case CART_ROM_ONLY:
		case CART_MBC3:
		case CART_MBC3_TIMER_BATT:
		case CART_MBC5:
			fatal(state, "invalid memory write at %04X (%02X)",
				location, data);
		case CART_MBC3_RAM:
		case CART_MBC3_RAM_BATT:
		case CART_MBC3_TIMER_RAM_BATT:
			state->ram_bank = data & 0x3;
			return;
		case CART_MBC5_RAM:
		case CART_MBC5_RAM_BATT:
			state->ram_bank = data & 0xF;
			return;
		default:
			fatal(state, "RAM banks for this cart (type %04X) aren't done yet sorry :(",
					state->cart_data[OFF_CART_TYPE]);
		}
	case 0x8:
	case 0x9:
		/* VRAM */
		vram_write(state, location, data);
		return;
	case 0xA:
	case 0xB:
		/* switched RAM bank */
		state->cart_ram[state->ram_bank][location - 0xA000] = data;
		return;
	case 0xE:
	case 0xF:
		switch(location >> 8)
		{
		case 0xFE:
			break;
		case 0xFF:
			if(location < 0xFF80)
			{
				hw_reg_write[location - 0xFF00](state, location, data);
				return;
			}
			else if(location == 0xFFFF)
			{
				int_mask_flag_write(state, data);
				return;
			}

			break;
		default:
			// who knows? the SHADOW knows! - 0xE000..0xFDFF
			location -= 0x2000;
		}
	}

	state->memory[location] = data;
}

void mem_write16(emu_state *restrict state, uint16_t location, uint16_t data)
{
        mem_write8(state, location++, (uint8_t)(data & 0xFF));
        mem_write8(state, location, (uint8_t)(data >> 8));
}
