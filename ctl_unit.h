#ifndef __CTL_UNIT_H__
#define __CTL_UNIT_H__

#include <stdint.h>	// uint[XX]_t

#include "sgherm.h"	// emulator_state
#include "params.h"	// system_types

#define WAIT_CYCLE(state, cycles, handler) \
	if(state->wait == 0) {\
		handler;\
		state->wait = cycles;\
	}\
	state->wait--;


/*! Zero Flag */
#define FLAG_Z 0x80
/*! Subtract Flag */
#define FLAG_N 0x40
/*! Half-Carry Flag */
#define FLAG_H 0x20
/*! Carry Flag */
#define FLAG_C 0x10


typedef enum
{
	CB_REG_B = 0,
	CB_REG_C,
	CB_REG_D,
	CB_REG_E,
	CB_REG_H,
	CB_REG_L,
	CB_REG_HL,
	CB_REG_A
} cb_regs;

typedef enum
{
	CB_OP_RLC = 0,
	CB_OP_RRC = 1,
	CB_OP_RL = 2,
	CB_OP_RR = 3,
	CB_OP_SLA = 4,
	CB_OP_SRA = 5,
	CB_OP_SWAP = 6,
	CB_OP_SRL = 7,
	CB_OP_BIT = 8,
	CB_OP_RES = 9,
	CB_OP_SET = 10
} cb_ops;


typedef void (*opcode_t)(emulator_state *restrict state);


void init_ctl(emulator_state *restrict state, system_types type);
bool execute(emulator_state *restrict);

uint8_t int_flag_read(emulator_state *restrict, uint16_t);
void int_flag_write(emulator_state *restrict, uint16_t, uint8_t);

#endif /*!__CTL_UNIT_H__*/
