#ifndef __SGHERM_H_
#define __SGHERM_H_

#include "config.h"	// macros

#include <stdbool.h>	// bool
#include <stdint.h>	// uint[XX]_t

// 8-bit address space
#define MEM_SIZE	0x10000


// XXX this doesn't belong here but it'll have to do for now
// (emu_state depends on it and everything depends on that)
typedef enum
{
	CPU_FREQ_GB = 4194304,
	CPU_FREQ_SGB = 4295454,
	CPU_FREQ_GBC = 8388608,
} cpu_freq;


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

	bool interrupts;	/*! Interrupts enabled */
} register_map;

typedef struct _lcdc_state
{
	uint32_t curr_clk;		/* current clock */
} lcdc;

typedef struct _ser_state
{
	uint16_t curr_clk;		/*! ticks passed */
	uint8_t in, out;		/*! in / out values */
	int8_t cur_bit;			/*! the current bit */
	bool enabled;			/*! transfer active */
	bool use_internal;		/*! clock source */
} ser;

typedef struct _snd_state
{
	struct _ch1
	{
		/*! channel enabled? */
		bool enabled;
		/*! this/128Hz = sweep */
		uint8_t sweep_time;
		/*! if true, sweep decreases frequency.
		 *  otherwise, sweep increases frequency. */
		bool sweep_dec;
		/*! number of shift */
		uint8_t shift;
		/*! wave pattern duty: 1=12.5%,2=25%,3=50%,4=75% */
		uint8_t wave_duty;
		/*! sound length */
		uint8_t length;
		/*! initial envelope volume */
		uint8_t envelope_volume;
		/*! if true, envelope amplifies.
		 *  otherwise, envelope attenuates. */
		bool envelope_amp;
		/*! number of sweeps (0 = stop) */
		uint8_t sweep;
		/*! if true, one-shot.  otherwise, loop */
		bool counter;
		/*! 11-bit frequency (higher 5 = nothing) */
		uint16_t frequency;
	} ch1;
	struct _ch2
	{
		bool enabled;		/*! channel enabled? */
	} ch2;
	struct _ch3
	{
		bool enabled;		/*! channel enabled? */
		uint8_t wave[16];	/*! waveform data */
	} ch3;
	struct _ch4
	{
		bool enabled;		/*! channel enabled? */
	} ch4;
	bool enabled;			/*! sound active? */
} snd;

typedef struct _tm_state
{
	uint8_t div;			/*! DIV register */
	uint8_t tima;			/*! TIMA register */
	uint8_t rounds;			/*! TMA register */
	uint16_t ticks_per_tima;	/*! ticks per TIMA++ */
	uint8_t curr_clk;		/*! ticks passed */
	bool enabled;			/*! timer armed */
} timer;

typedef struct _input_state
{
	uint8_t col_state;		/*! P14 and P15 */
	uint8_t row_state;		/*! P10 through P13 */
} input;

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

	// hardware
	lcdc lcdc_state;
	snd snd_state;
	timer timer_state;
	input input_state;
	ser ser_state;
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
