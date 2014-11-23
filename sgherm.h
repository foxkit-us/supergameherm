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

typedef struct _emu_state
{
	unsigned char memory[MEM_SIZE];		/*! RAM */
	unsigned char *cart_data;		/*! Cartridge data */

	struct _registers
	{
		uint16_t af, bc, de, hl, sp, pc;	/*! Registers */
		uint8_t *const a, *const f;		/*! Sub-registers */
		uint8_t *const b, *const c;		/*! Sub-registers */
		uint8_t *const d, *const e;		/*! Sub-registers */
		uint8_t *const h, *const l;		/*! Sub-registers */
		bool interrupts;			/*! Interrupts enabled */
	} registers;

	bool halt;				/*! waiting for interrupt */
	bool stop;				/*! deep sleep state (disable LCDC) */

	uint16_t dma_membar_wait;		/*! Clocks left on DMA membar */

	unsigned int wait;			/*! number of clocks */

	uint8_t bank;				/*! current bank */

	uint64_t cycles;			/*! Present cycle count */
	uint64_t start_time;			/*! Time started */
	cpu_freq freq;				/*! CPU frequency */

	struct _lcdc_state
	{
		uint32_t curr_clk;		/* current clock */
	} lcdc_state;

	struct _ser_state
	{
		uint16_t curr_clk;		/*! ticks passed */
		uint8_t in, out;		/*! in / out values */
		int8_t cur_bit;			/*! the current bit */
		bool enabled;			/*! transfer active */
		bool use_internal;		/*! clock source */
	} ser_state;

	struct _snd_state
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
	} snd_state;

	struct _tm_state
	{
		uint8_t div;			/*! DIV register */
		uint8_t tima;			/*! TIMA register */
		uint8_t rounds;			/*! TMA register */
		uint16_t ticks_per_tima;	/*! ticks per TIMA++ */
		uint8_t curr_clk;		/*! ticks passed */
		bool enabled;			/*! timer armed */
	} timer_state;

	struct _input_state
	{
		uint8_t col_state;		/*! P14 and P15 */
		uint8_t row_state;		/*! P10 through P13 */
	} input_state;
} emu_state;

emu_state * init_emulator(void);

#endif /*!__SGHERM_H_*/
