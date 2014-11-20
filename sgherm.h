#ifndef __SGHERM_H_
#define __SGHERM_H_

#include <stdbool.h>	// bool
#include <stdint.h>	// uint[XX]_t

/* 8-bit address space */
#define MEM_SIZE	0x10000

typedef struct _emulator_state
{
	unsigned char memory[MEM_SIZE];		/*! RAM */
	unsigned char *cart_data;		/*! Loaded cart data */
	uint16_t af, bc, de, hl, sp, pc;	/*! Registers */
	char flag_reg;
	bool disable_int_on_next;
	bool enable_int_on_next;
	bool interrupts;			/* Initalise to 1! */
	uint8_t wait;				/* number of clocks */
	struct _lcdc_state
	{
		uint32_t curr_clk;		/* current clock */
	} lcdc_state;
	struct _ser_state
	{
		uint8_t curr_clk;		/*! ticks passed */
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
	uint8_t bank;				/*! current bank */
} emulator_state;


/* XXX ugh */
#define REG_HI(state, reg) ((uint8_t *)&((state)->reg) + 1)
#define REG_LOW(state, reg) ((uint8_t *)&((state)->reg))

#define REG_A(state) REG_HI(state, af)
#define REG_F(state) REG_LOW(state, af)
#define REG_B(state) REG_HI(state, bc)
#define REG_C(state) REG_LOW(state, bc)
#define REG_D(state) REG_HI(state, de)
#define REG_E(state) REG_LOW(state, de)
#define REG_H(state) REG_HI(state, hl)
#define REG_L(state) REG_LOW(state, hl)

void init_emulator(emulator_state *restrict state);

#ifdef likely
#	undef likely
#endif

#ifdef __GNUC__
#	define likely(x) __builtin_expect((x), 1)
#else
#	define likely(x) (x)
#endif

#ifdef unlikely
#	undef unlikely
#endif

#ifdef __GNUC__
#	define unlikely(x) __builtin_expect((x), 0)
#else
#	define unlikely(x) (x)
#endif

#endif /*!__SGHERM_H_*/
