#ifndef __SGHERM_H_
#define __SGHERM_H_

#include "util.h"	// Necessary utilities

#include "lcdc.h"	// lcdc
#include "timer.h"	// cpu_freq
#include "serio.h"	// ser
#include "sound.h"	// snd
#include "input.h"	// input
#include "ctl_unit.h"	// interrupts
#include "frontend.h"	// frontend

#include <stdbool.h>	// bool
#include <stdint.h>	// uint[XX]_t


// 8-bit address space
#define MEM_SIZE	0x10000


typedef struct _registers
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
			uint8_t f, a;
			uint8_t c, b;
			uint8_t e, d;
			uint8_t l, h;
		} _8;
	} gp;

	uint16_t pc;		/*! Program counter */
	uint16_t sp;		/*! Stack pointer */
} register_map;

/*! The main emulation state structure */
typedef struct _emu_state
{
	unsigned char memory[MEM_SIZE];	/*! RAM */
	unsigned char *cart_data;	/*! Cartridge data */

	register_map registers;		/*! Registers */

	bool halt;			/*! waiting for interrupt */
	bool stop;			/*! deep sleep state (disable LCDC) */

	uint16_t dma_membar_wait;	/*! Clocks left on DMA membar */

	unsigned int wait;		/*! number of clocks */

	uint8_t bank;			/*! current ROM bank */
	uint8_t ram_bank;		/*! current RAM bank */

	uint64_t cycles;		/*! Present cycle count */
	uint64_t start_time;		/*! Time started */
	cpu_freq freq;			/*! CPU frequency */

	interrupts int_state;

	// hardware
	lcdc lcdc_state;
	snd snd_state;
	timer timer_state;
	input input_state;
	ser ser_state;

	frontend front;
} emu_state;


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

emu_state * init_emulator(void);

#endif /*!__SGHERM_H_*/
