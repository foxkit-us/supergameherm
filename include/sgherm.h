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
#include "mbc.h"	// mbc
#include "ctl_unit.h"	// interrupts
#include "frontend.h"	// frontend
#include "debug.h"	// debug_state


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

// Define a pair of 8-bit regs with appropriate endianness
#ifdef LITTLE_ENDIAN
#	define SUBREG(r1, r2) struct { uint8_t r2, r1; }
#else
#	define SUBREG(r1, r2) struct { uint8_t r1, r2; }
#endif

// Define a 16-bit register and its two 8-bit regs.
#define REGDEF(r1, r2) union { uint16_t r1##r2; SUBREG(r1, r2); }

struct registers_t
{
	REGDEF(a, f);		//! AF (A and Flags)
	REGDEF(b, c);		//! BC (B and C) registers
	REGDEF(d, e);		//! DE (D and E) registers
	REGDEF(h, l);		//! HL (H and L) registers

	uint16_t pc;		//! Program counter
	uint16_t sp;		//! Stack pointer
};

//! The main emulation state structure
struct emu_state_t
{
	uint8_t hram[0x7F];		//! High memory
	uint8_t wram[8][0x1000];	//! Work RAM banks (1-7 CGB only)
	uint_fast32_t wram_bank;	//! current WRAM bank

	uint8_t *cart_data;		//! Cartridge data
	uint_fast32_t cart_size;	//! Size of cartridge data
	const char *save_path;		//! Save file

	uint8_t *bootrom_data;		//! Exactly what it says on the tin
	bool in_bootrom;		//! Executing in bootrom
	uint_fast16_t bootrom_size;	//! Size of bootrom

	bool halt;			//! waiting for interrupt
	bool stop;			//! deep sleep state (disable LCDC)
	bool key1;			//! CGB speed switch

	uint_fast32_t dma_wait;		//! Clocks left on DMA membar

	uint_fast32_t wait;		//! number of clocks to wait

	uint_fast64_t cycles;		//! Present cycle count
	uint_fast64_t start_time;	//! Time started
	uint_fast64_t next_vblank_time;	//! Timestamp for next vblank

	system_types system;		//! Present emulation mode

	// CPU state
	cpu_freq freq;			//! CPU frequency
	interrupt_state interrupts;
	register_state registers;	//! Registers

	// hardware
	mbc_state mbc;
	lcdc_state lcdc;
	snd_state snd;
	timer_state timer;
	input_state input;
	ser_state ser;

	debug_state debug;

	frontend front;
};


// Register accesses
#define REG_AF(state) ((state)->registers.af)
#define REG_BC(state) ((state)->registers.bc)
#define REG_DE(state) ((state)->registers.de)
#define REG_HL(state) ((state)->registers.hl)
#define REG_PC(state) ((state)->registers.pc)
#define REG_SP(state) ((state)->registers.sp)

#define REG_A(state) ((state)->registers.a)
#define REG_F(state) ((state)->registers.f)
#define REG_B(state) ((state)->registers.b)
#define REG_C(state) ((state)->registers.c)
#define REG_D(state) ((state)->registers.d)
#define REG_E(state) ((state)->registers.e)
#define REG_H(state) ((state)->registers.h)
#define REG_L(state) ((state)->registers.l)

#define FLAG_SET(state, flag) (REG_F(state) |= (flag))
#define FLAG_UNSET(state, flag) (REG_F(state) &= ~(flag))
#define FLAG_FLIP(state, flag) (REG_F(state) ^= (flag))
#define FLAGS_OVERWRITE(state, value) (REG_F(state) = value)
#define FLAGS_CLEAR(state) FLAGS_OVERWRITE(state, 0)

#define IS_FLAG(state, flag) ((REG_F(state) & (flag)) == flag)

emu_state * init_emulator(const char *, const char *, const char *);
void finish_emulator(emu_state * restrict);
bool step_emulator(emu_state * restrict);

#endif /*!__SGHERM_H_*/
