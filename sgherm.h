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
	bool toggle_int_on_next;
	bool interrupts;			/* Initalise to 1! */
	uint8_t wait;				/* number of clocks */
	void *timer_obj;			/* Timer object */
} emulator_state;


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

void init_emulator(emulator_state *state);

#ifdef likely
#	undef likely
#	ifdef __GNUC__
#		define likely(x) __builtin_expect ((x), 1)
#	else
#		define likely(x) x
#	endif
#endif

#ifdef unlikely
#	undef unlikely
#	ifdef __GNUC__
#		define unlikely(x) __builtin_expect ((x), 0)
#	else
#		define unlikely(x) x
#	endif
#endif

#endif /*!__SGHERM_H_*/
