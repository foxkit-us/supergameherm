#ifndef __SGHERM_H_
#define __SGHERM_H_

#include <stdbool.h>	// bool
#include <stdint.h>	// uint[XX]_t

/* 8-bit address space */
#define MEM_SIZE	0x10000

#define I_DISABLE_INT_ON_NEXT	0x1
#define I_ENABLE_INT_ON_NEXT	0x2
#define I_INTERRUPTS		0x4

typedef struct _emulator_state
{
	unsigned char memory[MEM_SIZE];		/*! RAM */
	unsigned char *cart_data;		/*! Loaded cart data */

	struct _registers
	{
		uint16_t af, bc, de, hl, sp, pc;	/*! Registers */
		uint8_t *const a;
		uint8_t *const f;
		uint8_t *const b;
		uint8_t *const c;
		uint8_t *const d;
		uint8_t *const e;
		uint8_t *const h;
		uint8_t *const l;
		uint8_t flags;
	} registers;

	uint8_t iflags;				/* Interrupt information */

	uint8_t wait;				/* number of clocks */

	uint8_t bank;				/*! current bank */

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
} emulator_state;

emulator_state * init_emulator(void);

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
