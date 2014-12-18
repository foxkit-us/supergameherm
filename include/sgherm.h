#ifndef __SGHERM_H_
#define __SGHERM_H_

#include "config.h"	// bool, uint[XX]_t
#include "typedefs.h"	// typedefs
#include "util.h"	// Necessary utilities

#include "lcdc.h"	// lcdc
#include "timer.h"	// cpu_freq
#include "serio.h"	// ser
#include "sound.h"	// snd
#include "input.h"	// input
#include "ctl_unit.h"	// interrupts
#include "frontend.h"	// frontend


typedef enum
{
	SYSTEM_DMG,
	SYSTEM_MGB,
	SYSTEM_MGL,
	SYSTEM_SGB,
	SYSTEM_SGB2,
	SYSTEM_CGB
} system_types;

// 8-bit address space
#define MEM_SIZE	0x10000

struct registers_t
{
	/*! The 8/16 bit registers */
	union
	{
		/*! 16-bit registers */
		struct __16
		{
			uint16_t af;
			uint16_t bc;
			uint16_t de;
			uint16_t hl;
		} _16;
		/*! 8-bit registers shadowing the 16-bit */
		struct __8
		{
#ifdef BIG_ENDIAN
			uint8_t a, f;
			uint8_t b, c;
			uint8_t d, e;
			uint8_t h, l;
#else
			uint8_t f, a;
			uint8_t c, b;
			uint8_t e, d;
			uint8_t l, h;
#endif
		} _8;
	} gp;

	uint16_t pc;		/*! Program counter */
	uint16_t sp;		/*! Stack pointer */
};

/*! The main emulation state structure */
struct emu_state_t
{
	uint8_t memory[MEM_SIZE];	/*! RAM */
	uint8_t *cart_data;		/*! Cartridge data */

	/*! Cartridge RAM */
	uint8_t cart_ram[0xF][0x2000];

	register_state registers;	/*! Registers */

	bool halt;			/*! waiting for interrupt */
	bool stop;			/*! deep sleep state (disable LCDC) */

	uint_fast16_t dma_wait;	/*! Clocks left on DMA membar */

	uint_fast32_t wait;		/*! number of clocks */

	uint_fast16_t bank;		/*! current ROM bank */
	uint_fast8_t ram_bank;		/*! current RAM bank */

	uint_fast32_t cycles;		/*! Present cycle count */
	uint64_t start_time;		/*! Time started */
	uint64_t last_vblank_time;		/*! Last time since a million cycles */

	system_types system;		/*! Present emulation mode */
	cpu_freq freq;			/*! CPU frequency */

	interrupt_state interrupts;

	// hardware
	lcdc_state lcdc;
	snd_state snd;
	timer_state timer;
	input_state input;
	ser_state ser;

	frontend front;
};


// Register accesses
#define REG_16(state, reg) ((state)->registers.gp._16.reg)
#define REG_8(state, reg) ((state)->registers.gp._8.reg)

#define REG_AF(state) REG_16(state, af)
#define REG_BC(state) REG_16(state, bc)
#define REG_DE(state) REG_16(state, de)
#define REG_HL(state) REG_16(state, hl)
#define REG_PC(state) ((state)->registers.pc)
#define REG_SP(state) ((state)->registers.sp)

#define REG_A(state) REG_8(state, a)
#define REG_F(state) REG_8(state, f)
#define REG_B(state) REG_8(state, b)
#define REG_C(state) REG_8(state, c)
#define REG_D(state) REG_8(state, d)
#define REG_E(state) REG_8(state, e)
#define REG_H(state) REG_8(state, h)
#define REG_L(state) REG_8(state, l)

#define FLAG_SET(state, flag) (REG_F(state) |= (flag))
#define FLAG_UNSET(state, flag) (REG_F(state) &= ~(flag))
#define FLAG_FLIP(state, flag) (REG_F(state) ^= (flag))
#define FLAGS_OVERWRITE(state, value) (REG_F(state) = value)
#define FLAGS_CLEAR(state) FLAGS_OVERWRITE(state, 0)

#define IS_FLAG(state, flag) ((REG_F(state) & (flag)) == flag)

emu_state * init_emulator(const char *);
void finish_emulator(emu_state * restrict);
bool step_emulator(emu_state * restrict);

#endif /*!__SGHERM_H_*/
