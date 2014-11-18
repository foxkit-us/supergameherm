#ifndef __SGHERM_H_
#define __SGHERM_H_

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
} emulator_state;


#define REG_HI(state, reg) ((char *)&((state)->reg) + 1)
#define REG_LOW(state, reg) ((char *)&((state)->reg))

#define REG_A(state) REG_HI(state, af)
#define REG_F(state) REG_LOW(state, af)
#define REG_B(state) REG_HI(state, bc)
#define REG_C(state) REG_LOW(state, bc)
#define REG_D(state) REG_HI(state, de)
#define REG_E(state) REG_LOW(state, de)
#define REG_H(state) REG_HI(state, hl)
#define REG_L(state) REG_LOW(state, hl)

#endif /*!__SGHERM_H_*/
