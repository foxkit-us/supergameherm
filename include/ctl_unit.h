#ifndef __CTL_UNIT_H__
#define __CTL_UNIT_H__

#include "memory.h"	// mem_write8

#include <stdbool.h>	// bool
#include <stdint.h>	// uint[XX]_t


/*! Zero Flag */
#define FLAG_Z 0x80
/*! Subtract Flag */
#define FLAG_N 0x40
/*! Half-Carry Flag */
#define FLAG_H 0x20
/*! Carry Flag */
#define FLAG_C 0x10

// Interrupt flags
#define INT_VBLANK	0x1
#define INT_LCD_STAT	0x2
#define INT_TIMER	0x4
#define INT_SERIAL	0x8
#define INT_JOYPAD	0x10


typedef enum
{
	SYSTEM_DMG,
	SYSTEM_MGB,
	SYSTEM_MGL,
	SYSTEM_SGB,
	SYSTEM_SGB2,
	SYSTEM_CGB
} system_types;


typedef enum
{
	INT_ID_NONE = 0x0000,
	INT_ID_VBLANK = 0x0040,
	INT_ID_LCD_STAT = 0x0048,
	INT_ID_TIMER = 0x0050,
	INT_ID_SERIAL = 0x0058,
	INT_ID_JOYPAD = 0x0060
} /* Ralf Brown's */ interrupt_list;


typedef enum
{
	INT_NEXT_NONE = 0,
	INT_NEXT_ENABLE,
	INT_NEXT_DISABLE,
} interrupt_next;


typedef struct _interrupt_state
{
	interrupt_next next_cycle;	/*! Interrupts will be enabled next cycle */
	bool enabled;			/*! Interrupts enabled */
	uint8_t mask;			/*! Interrupt mask */
	uint8_t pending;		/*! Pending interrupts */

	// Optimisation - cache the next interrupt jump address
	interrupt_list next_jmp;
} interrupts;


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


typedef void (*opcode_t)(emu_state *restrict state);


void init_ctl(emu_state *restrict state);
bool execute(emu_state *restrict);

uint8_t int_flag_read(emu_state *restrict, uint16_t);
void int_flag_write(emu_state *restrict, uint16_t, uint8_t);
void int_mask_flag_write(emu_state *restrict state, uint8_t data);

#define signal_interrupt(state, interrupt) \
	mem_write8((state), 0xFF0F, (interrupt) | ((state)->int_state.pending))

#endif /*!__CTL_UNIT_H__*/
