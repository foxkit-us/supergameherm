#ifndef __SGHERM_H_
#define __SGHERM_H_

#include "config.h"	// macros

#include <stdbool.h>	// bool
#include <stdint.h>	// uint[XX]_t

// 8-bit address space
#define MEM_SIZE	0x10000

// Interrupt flags
#define I_DISABLE_INT_ON_NEXT	0x1
#define I_ENABLE_INT_ON_NEXT	0x2
#define I_INTERRUPTS		0x4


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
		uint8_t *const a, *const f;
		uint8_t *const b, *const c;
		uint8_t *const d, *const e;
		uint8_t *const h, *const l;
		uint8_t flags;
	} registers;

	unsigned int iflags;			/* Interrupt information */

	unsigned int wait;			/* number of clocks */

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

	struct _tm_state
	{
		uint8_t div;			/*! DIV register */
		uint8_t tima;			/*! TIMA register */
		uint8_t rounds;			/*! TMA register */
		uint16_t ticks_per_tima;	/*! ticks per TIMA++ */
		uint8_t curr_clk;		/*! ticks passed */
		bool enabled;			/*! timer armed */
	} timer_state;
} emu_state;

emu_state * init_emulator(void);

#ifdef unused
#	undef unused
#endif

#ifdef likely
#	undef likely
#endif

#ifdef unlikely
#	undef unlikely
#endif

#ifdef __GNUC__
#	define unused __attribute__((unused))
#	define unlikely(x) __builtin_expect((x), 0)
#	define likely(x) __builtin_expect((x), 1)
#else
#	define unused
#	define unlikely(x) (x)
#	define likely(x) (x)
#endif

#endif /*!__SGHERM_H_*/
