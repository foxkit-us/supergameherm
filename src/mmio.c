#include "config.h"	// macros, uint[XX]_t
#include <assert.h>	// assert
#include <string.h>	// memmove
#include <stdlib.h>	// free

#include "sgherm.h"	// emu_state
#include "ctl_unit.h"	// int_flag_*
#include "input.h"	// joypad_*
#include "lcdc.h"	// lcdc_read
#include "memory.h"	// Constants and what have you
#include "print.h"	// fatal
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
static inline uint8_t no_hw_read(emu_state *restrict state UNUSED, uint16_t location)
{
	warning(state, "no device present at %04X (emulator bug? incompatible GB?)", location);
	return 0xFF;
}

/***********************************************************************
 * writers
 ***********************************************************************/
typedef void (*mem_write8_fn)(emu_state *restrict , uint16_t, uint8_t);

/*!
 * @brief silly name, decent intent
 * @result emulation terminates because some doofus wrote to a r/o reg
 */
static inline void readonly_reg_write(emu_state *restrict state, uint16_t location, uint8_t data)
{
	warning(state, "[%4X] attempted write of %02X to read-only register " \
		"%04X (a real GB ignores this)", REG_PC(state), data,
		location);
}

/*!
 * @brief what happens when you poke around randomly
 * @result emulation terminates because some doofus wrote to a device not there
 */
static inline void no_hw_write(emu_state *restrict state, uint16_t location, uint8_t data)
{
	warning(state, "[%4X] attempted doofus write of %02X to non-existent " \
		"device at %04X (a real GB ignores this)",
		REG_PC(state), data, location);
}


// Device MMIO
#include "mmio_input.c"
#include "mmio_interrupt.c"
#include "mmio_lcdc.c"
#include "mmio_misc.c"
#include "mmio_serio.c"
#include "mmio_sound.c"
#include "mmio_timer.c"


//! a table of hardware register read methods
static mem_read_fn hw_reg_read[0x80] =
{
	joypad_read, // 00 - P1 - joypad
	serial_read, // 01 - SB - serial data
	serial_read, // 02 - SC - serial control
	no_hw_read, // 03 - NO HARDWARE
	timer_read,  // 04 - DIV
	timer_read,  // 05 - TIMA - timer step
	timer_read,  // 06 - TMA - timer count
	timer_read,  // 07 - TAC - timer frequency / enable

	// 08..0E - NO HARDWARE
	no_hw_read, no_hw_read, no_hw_read, no_hw_read,
	no_hw_read, no_hw_read, no_hw_read,

	int_flag_read, // 0F - IF - interrupt status.. also CPU-based

	// 10..14 - sound mode 1
	sound_read, sound_read, sound_read, sound_read, sound_read,

	no_hw_read, // 15 - NO HARDWARE

	// 16..19 - sound mode 2
	sound_read, sound_read, sound_read, sound_read,
	// 1A..1E - sound mode 3
	sound_read, sound_read, sound_read, sound_read, sound_read,

	no_hw_read, // 1F - NO HARDWARE

	// 20..23 - sound mode 4
	sound_read, sound_read, sound_read, sound_read,
	// 24..26 - sound control
	sound_read, sound_read, sound_read,

	// 27..2F - NO HARDWARE
	no_hw_read, no_hw_read, no_hw_read, no_hw_read, no_hw_read,
	no_hw_read, no_hw_read, no_hw_read, no_hw_read,

	// 30..3F - WAV RAM
	sound_read, sound_read, sound_read, sound_read, sound_read, sound_read,
	sound_read, sound_read, sound_read, sound_read, sound_read, sound_read,
	sound_read, sound_read, sound_read, sound_read,

	// 40..45 - LCD controller
	lcdc_control_read, lcdc_stat_read, lcdc_scroll_read,
	lcdc_scroll_read, lcdc_ly_read, lcdc_lyc_read,

	no_hw_read, // 46 - DMA - DMA transfer and control

	// 47..4B - more graphics stuff
	lcdc_bgp_read, lcdc_objp_read, lcdc_objp_read,
	lcdc_window_read, lcdc_window_read,

	// 4C - NO HARDWARE
	no_hw_read,

	// 4D - KEY1 - switch clock rate (CGB only)
	key1_read,

	// 4E - NO HARDWARE
	no_hw_read,

	// 4F - switch VRAM bank (CGB only)
	vram_bank_switch_read,

	// 50..54 - NO HARDWARE
	no_hw_read, no_hw_read, no_hw_read, no_hw_read, // 0x53
	no_hw_read, // 0x54

	// 55 - HDMA status (CGB only)
	hdma_status_read,

	// 56..67 - NO HARDWARE
	no_hw_read, no_hw_read, // 0x57
	no_hw_read, no_hw_read, no_hw_read, no_hw_read, // 0x5B
	no_hw_read, no_hw_read, no_hw_read, no_hw_read, // 0x5F
	no_hw_read, no_hw_read, no_hw_read, no_hw_read, // 0x63
	no_hw_read, no_hw_read, no_hw_read, no_hw_read, // 0x67

	// 68..6B - CGB palette stuff (nothing on GB)
	bg_pal_ind_read, bg_pal_data_read, sprite_pal_ind_read, sprite_pal_data_read,

	// 6C..7F - NO HARDWARE
	no_hw_read, no_hw_read, no_hw_read, no_hw_read, // 0x6F
	no_hw_read, no_hw_read, no_hw_read, no_hw_read, // 0x73
	no_hw_read, no_hw_read, no_hw_read, no_hw_read, // 0x77
	no_hw_read, no_hw_read, no_hw_read, no_hw_read, // 0x7B
	no_hw_read, no_hw_read, no_hw_read, no_hw_read  // 0x7F
};

static mem_write8_fn hw_reg_write[0x80] =
{
	joypad_write, // 00 - P1 - joypad
	serial_write, // 01 - SB - serial data
	serial_write, // 02 - SC - serial control
	no_hw_write, // 03 - NO HARDWARE
	timer_write,  // 04 - DIV - CPU register
	timer_write,  // 05 - TIMA - timer step
	timer_write,  // 06 - TMA - timer count
	timer_write,  // 07 - TAC - timer control

	// 08..0E - NO HARDWARE
	no_hw_write, no_hw_write, no_hw_write, no_hw_write,
	no_hw_write, no_hw_write, no_hw_write,

	int_flag_write, // 0F - IF - interrupt control

	// 10..14 - sound bank 1
	sound_write, sound_write, sound_write, sound_write,
	sound_write,

	no_hw_write, // 15 - NO HARDWARE

	// 16..19 - sound bank 2
	sound_write, sound_write, sound_write, sound_write,

	// 1A..1E - sound bank 3
	sound_write, sound_write, sound_write, sound_write,
	sound_write,

	no_hw_write, // 1F - NO HARDWARE

	// 20..23 - sound bank 4
	sound_write, sound_write, sound_write, sound_write,

	// 24..26 - sound control
	sound_write, sound_write, sound_write,

	// 27..2F - NO HARDWARE
	no_hw_write, no_hw_write, no_hw_write,
	no_hw_write, no_hw_write, no_hw_write,
	no_hw_write, no_hw_write, no_hw_write,

	// 30..3F - WAV RAM
	sound_write, sound_write, sound_write, sound_write,
	sound_write, sound_write, sound_write, sound_write,
	sound_write, sound_write, sound_write, sound_write,
	sound_write, sound_write, sound_write, sound_write,

	// 40..45 - LCD controller
	lcdc_control_write, lcdc_stat_write, lcdc_scroll_write,
	lcdc_scroll_write, lcdc_ly_write, lcdc_lyc_write,

	dma_write, // 46 - DMA

	// 47..4B - more video
	lcdc_bgp_write, lcdc_objp_write, lcdc_objp_write,
	lcdc_window_write, lcdc_window_write,

	// 4C - NO HARDWARE
	no_hw_write,

	// 4D - KEY1 - switch clock rate (CGB only)
	key1_write,

	// 4E - NO HARDWARE
	no_hw_write,

	// 4F - VRAM bank switch (CGB only)
	vram_bank_switch_write,

	// 50 - exit boot ROM
	bootrom_exit,

	// 51..55 - HDMA
	hdma_reg_write,
	hdma_reg_write,
	hdma_reg_write,
	hdma_reg_write,
	hdma_reg_write,

	// 51..67 - NO HARDWARE
	no_hw_write, no_hw_write,	// 0x57
	no_hw_write, no_hw_write, no_hw_write, no_hw_write,	// 0x5B
	no_hw_write, no_hw_write, no_hw_write, no_hw_write,	// 0x5F
	no_hw_write, no_hw_write, no_hw_write, no_hw_write,	// 0x63
	no_hw_write, no_hw_write, no_hw_write, no_hw_write,	// 0x67

	// 68..6B - CGB palette stuff (nothing on GB)
	bg_pal_ind_write, bg_pal_data_write, sprite_pal_ind_write, sprite_pal_data_write,

	// 6C..6F - NO HARDWARE
	no_hw_write, no_hw_write, no_hw_write, no_hw_write, // 0x6F

	// 70 - WRAM bank switch (CGB only)
	wram_bank_switch_write,

	// 71..7F - NO HARDWARE
	no_hw_write, no_hw_write, no_hw_write, // 0x73
	no_hw_write, no_hw_write, no_hw_write, no_hw_write, // 0x77
	no_hw_write, no_hw_write, no_hw_write, no_hw_write, // 0x7B
	no_hw_write, no_hw_write, no_hw_write, no_hw_write, // 0x7F
};

uint8_t hw_read(emu_state *restrict state, uint16_t location)
{
	return hw_reg_read[location & 0xFF](state, location);
}

void hw_write(emu_state *restrict state, uint16_t location, uint8_t data)
{
	hw_reg_write[location & 0xFF](state, location, data);
}

