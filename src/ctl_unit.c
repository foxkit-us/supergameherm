#include "config.h"		// bool, integer types

#include "sgherm.h"		// emu_state, etc.
#include "util_bitops.h"	// bit twiddling
#include "ctl_unit.h"		// prototypes, constants, etc.
#include "debug.h"		// state dumps etc
#include "print.h"		// fatal

#include <assert.h>		// assert
#include <stdlib.h>		// NULL
#include <string.h>		// memcpy


void compute_irq(emu_state *restrict state)
{
	if(state->interrupts.enabled)
	{
		state->interrupts.irq = state->interrupts.pending & state->interrupts.mask & 0x1F;
	}
}

void signal_interrupt(emu_state *restrict state, int interrupt)
{
	state->interrupts.pending |= interrupt;
	state->halt = state->stop = false;
	compute_irq(state);
}

uint8_t int_flag_read(emu_state *restrict state, uint16_t location UNUSED)
{
	return state->interrupts.pending;
}

void int_flag_write(emu_state *restrict state, uint16_t location UNUSED, uint8_t data)
{
	state->interrupts.pending = data;
	compute_irq(state);
}

uint8_t int_mask_flag_read(emu_state *restrict state, uint16_t location UNUSED)
{
	return state->interrupts.mask;
}

void int_mask_flag_write(emu_state *restrict state, uint8_t data)
{
	state->interrupts.mask = data;
	compute_irq(state);
}

#include "instr_alu_arith.c"
#include "instr_alu_logic.c"
#include "instr_branch.c"
#include "instr_intr.c"
#include "instr_ld.c"
#include "instr_misc.c"
#include "instr_stack.c"


//! Bit length of given instructions
static const int instr_len[0x100] =
{
	1, 3, 1, 1, 1, 1, 2, 1, 3, 1, 1, 1, 1, 1, 2, 1,		// 0x00
	2, 3, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1,		// 0x10
	2, 3, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1,		// 0x20
	2, 3, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1,		// 0x30
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 0x40
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 0x50
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 0x60
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 0x70
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 0x80
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 0x90
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 0xA0
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 0xB0
	1, 1, 3, 3, 3, 1, 2, 1, 1, 1, 3, 2, 3, 3, 2, 1,		// 0xC0
	1, 1, 3, 0, 3, 1, 2, 1, 1, 1, 3, 0, 3, 0, 2, 1,		// 0xD0
	2, 1, 1, 0, 0, 1, 2, 1, 2, 1, 3, 0, 0, 0, 2, 1,		// 0xE0
	2, 1, 1, 1, 0, 1, 2, 1, 2, 1, 3, 1, 0, 0, 2, 1,		// 0xF0
};

static const opcode_t handlers[0x100] =
{
	nop, ld_bc_d16, ld_bc_a, inc_bc, inc_b, dec_b, ld_b_d8, rlca,		// 0x00
	ld_d16_sp, add_hl_bc, ld_a_bc, dec_bc, inc_c, dec_c, ld_c_d8, rrca,	// 0x08
	stop, ld_de_d16, ld_de_a, inc_de, inc_d, dec_d, ld_d_d8, rla,		// 0x10
	jr_r8, add_hl_de, ld_a_de, dec_de, inc_e, dec_e, ld_e_d8, rra,		// 0x18
	jr_nz_r8, ld_hl_d16, ldi_hl_a, inc_hl, inc_h, dec_h, ld_h_d8, daa,	// 0x20
	jr_z_r8, add_hl_hl, ldi_a_hl, dec_hl, inc_l, dec_l, ld_l_d8, cpl,	// 0x28
	jr_nc_r8, ld_sp_d16, ldd_hl_a, inc_sp, inc_hl_mem, dec_hl_mem, ld_hl_d8, scf,// 0x30
	jr_c_r8, add_hl_sp, ldd_a_hl, dec_sp, inc_a, dec_a, ld_a_d8, ccf,	// 0x38
	ld_b_b, ld_b_c, ld_b_d, ld_b_e, ld_b_h, ld_b_l, ld_b_hl, ld_b_a,	// 0x40
	ld_c_b, ld_c_c, ld_c_d, ld_c_e, ld_c_h, ld_c_l, ld_c_hl, ld_c_a,	// 0x48
	ld_d_b, ld_d_c, ld_d_d, ld_d_e, ld_d_h, ld_d_l, ld_d_hl, ld_d_a,	// 0x50
	ld_e_b, ld_e_c, ld_e_d, ld_e_e, ld_e_h, ld_e_l, ld_e_hl, ld_e_a,	// 0x58
	ld_h_b, ld_h_c, ld_h_d, ld_h_e, ld_h_h, ld_h_l, ld_h_hl, ld_h_a,	// 0x60
	ld_l_b, ld_l_c, ld_l_d, ld_l_e, ld_l_h, ld_l_l, ld_l_hl, ld_l_a,	// 0x68
	ld_hl_b, ld_hl_c, ld_hl_d, ld_hl_e, ld_hl_h, ld_hl_l, halt, ld_hl_a,	// 0x70
	ld_a_b, ld_a_c, ld_a_d, ld_a_e, ld_a_h, ld_a_l, ld_a_hl, ld_a_a,	// 0x78
	add_b, add_c, add_d, add_e, add_h, add_l, add_hl, add_a,		// 0x80
	adc_b, adc_c, adc_d, adc_e, adc_h, adc_l, adc_hl, adc_a,		// 0x88
	sub_b, sub_c, sub_d, sub_e, sub_h, sub_l, sub_hl, sub_a,		// 0x90
	sbc_b, sbc_c, sbc_d, sbc_e, sbc_h, sbc_l, sbc_hl, sbc_a,		// 0x98
	and_b, and_c, and_d, and_e, and_h, and_l, and_hl, and_a,		// 0xA0
	xor_b, xor_c, xor_d, xor_e, xor_h, xor_l, xor_hl, xor_a,		// 0xA8
	or_b, or_c, or_d, or_e, or_h, or_l, or_hl, or_a,			// 0xB0
	cp_b, cp_c, cp_d, cp_e, cp_h, cp_l, cp_hl, cp_a,			// 0xB8
	retnz, pop_bc, jp_nz_d16, jp_d16, call_nz_d16, push_bc, add_d8, rst,	// 0xC0
	retz, ret, jp_z_d16, cb_dispatch, call_z_d16, call_d16, adc_d8, rst,	// 0xC8
	retnc, pop_de, jp_nc_d16, hcf, call_nc_d16, push_de, sub_d8, rst,	// 0xD0
	retc, reti, jp_c_d16, hcf, call_c_d16, hcf, sbc_d8, rst,		// 0xD8
	ldh_d8_a, pop_hl, ld_ff00_c_a, hcf, hcf, push_hl, and_d8, rst,		// 0xE0
	add_sp_d8, jp_hl, ld_d16_a, hcf, hcf, hcf, xor_d8, rst,			// 0xE8
	ldh_a_d8, pop_af, ld_a_ff00_c, di, hcf, push_af, or_d8, rst,		// 0xF0
	ld_hl_sp_d8, ld_sp_hl, ld_a_d16, ei, hcf, hcf, cp_d8, rst		// 0xF8
};


//! boot up
void init_ctl(emu_state *restrict state)
{
	REG_PC(state) = 0x0100;
	switch(state->system)
	{
	case SYSTEM_SGB:
		debug(state, "Super Game Boy emulation");
		REG_A(state) = 0x01;
		break;
	case SYSTEM_CGB:
		debug(state, "Game Boy Color emulation");
		REG_A(state) = 0x11;
		break;
	default:
		debug(state, "original Game Boy emulation");
		REG_A(state) = 0x01;
		break;
	}
	FLAGS_OVERWRITE(state, 0xB0);
	REG_B(state) = 0x00;
	REG_C(state) = 0x13;
	REG_D(state) = 0x00;
	REG_E(state) = 0xD8;
	REG_H(state) = 0x01;
	REG_L(state) = 0x4D;
	REG_SP(state) = 0xFFFE;

	state->debug.debug = true;
}


//! Call the needed interrupt handler
static inline void call_interrupt(emu_state *restrict state)
{
	uint8_t mask = state->interrupts.irq;
	uint8_t jump;

	assert(state->interrupts.mask != 0);
	assert(state->interrupts.pending != 0);

	if(mask & INT_VBLANK)
	{
		jump = INT_ID_VBLANK;
		state->interrupts.pending &= ~INT_VBLANK;
	}
	else if(mask & INT_LCD_STAT)
	{
		jump = INT_ID_LCD_STAT;
		state->interrupts.pending &= ~INT_LCD_STAT;
	}
	else if(mask & INT_TIMER)
	{
		jump = INT_ID_TIMER;
		state->interrupts.pending &= ~INT_TIMER;
	}
	else if(mask & INT_SERIAL)
	{
		jump = INT_ID_SERIAL;
		state->interrupts.pending &= ~INT_SERIAL;
	}
	else if(mask & INT_JOYPAD)
	{
		jump = INT_ID_JOYPAD;
		state->interrupts.pending &= ~INT_JOYPAD;
	}
	else
	{
		return;
	}

	// Push pc to the stack
	REG_SP(state) -= 2;
	mem_write16(state, REG_SP(state), REG_PC(state));

	// Jump!
	REG_PC(state) = jump;

	// Reset these states
	state->halt = state->stop = false;

	state->wait = 20;

	// Interrupts are locked out before handling
	state->interrupts.enabled = false;
	state->interrupts.irq = 0;
}

#ifndef NDEBUG
static inline void dump_all_state_invalid_flag(emu_state *state, uint8_t opcode,
		uint8_t cb, uint16_t pc_prev, uint8_t flags_prev)
{
	dump_all_state(state);

	if(cb == 0)
	{
		debug(state, "opcode: %02X [mnemonic: %s] pc_prev: %04X flags_prev: %04X",
			opcode, mnemonics[opcode], pc_prev, flags_prev);
	}
	else
	{
		debug(state, "opcode: %02X [cb %02X] [mnemonic: %s] pc_prev: %04X flags_prev: %04X",
			opcode, cb, mnemonics_cb[cb], pc_prev, flags_prev);
	}
}
#endif

//! the emulated CU for the 'z80-ish' CPU
bool execute(emu_state *restrict state)
{
	uint8_t opcode;
	uint8_t op_data[2] = {0xBE, 0xEF};
	int op_len;
	opcode_t handler;
#ifndef NDEBUG
	uint16_t pc_prev = REG_PC(state);
	uint8_t flags_prev = REG_F(state);
	uint8_t cb = 0;
	const char *flag_req;
#endif

	if(state->wait)
	{
		state->wait--;
		return true;
	}

	if(unlikely(state->dma_wait))
	{
		state->dma_wait--;

		// Double speed
		if(state->freq == CPU_FREQ_CGB)
		{
			state->dma_wait--;
		}
	}

	// Check for interrupts
	if(state->interrupts.irq)
	{
		call_interrupt(state);
	}


	switch(state->interrupts.enable_ctr)
	{
	case 2:
		state->interrupts.enable_ctr--;
		break;
	case 1:
		state->interrupts.enable_ctr = 0;
		state->interrupts.enabled = true;
		compute_irq(state);
		break;
	}

	if(state->halt || state->stop)
	{
		// Waiting for an interrupt
		return true;
	}

	opcode = mem_read8(state, REG_PC(state)++);
	op_len = instr_len[opcode] - 1;

	if(op_len > 0)
	{
		int i = 0;
		for(; i < op_len; i++)
		{
			op_data[i] = mem_read8(state, REG_PC(state)++);
		}
	}

	// Copy last instructions
	if(state->debug.debug)
	{
		state->debug.last_opcode = opcode;
		memcpy(state->debug.last_param, op_data, sizeof(op_data));

		if(state->debug.instr_dump)
		{
			dump_state_pc(state, REG_PC(state) - op_len);
			debug(state, "INSTR: [%04X] %s\t\t[%02X %02X] (af=%04X bc=%04X de=%04X hl=%04X sp=%04X)",
				REG_PC(state) - op_len,
				opcode == 0xCB ? mnemonics_cb[opcode] : mnemonics[opcode],
				op_data[1], op_data[0],
				REG_AF(state), REG_BC(state), REG_DE(state), REG_HL(state), REG_SP(state));
		}
	}

#ifndef NDEBUG
	if(opcode == 0xCB)
	{
		cb = op_data[0];
		flag_req = flags_cb_expect[cb];
	}
	else
	{
		flag_req = flags_expect[opcode];
	}
#endif /*NDEBUG*/

	handler = handlers[opcode];
	handler(state, op_data);

#ifndef NDEBUG
	// Flag assertions
	switch(flag_req[0])
	{
	case '-':
		// Check to see if it has changed
		if((REG_F(state) ^ flags_prev) & FLAG_Z)
		{
			dump_all_state_invalid_flag(state, opcode, cb, pc_prev, flags_prev);
			fatal(state, "Flag Z changed when it wasn't supposed to");
		}

		break;
	case '0':
		// Check to ensure the flag has been cleared
		if(IS_FLAG(state, FLAG_Z))
		{
			dump_all_state_invalid_flag(state, opcode, cb, pc_prev, flags_prev);
			fatal(state, "Flag Z is set when it is NOT supposed to be");
		}

		break;
	case '1':
		// Check to ensure the flag has been set
		if(!IS_FLAG(state, FLAG_Z))
		{
			dump_all_state_invalid_flag(state, opcode, cb, pc_prev, flags_prev);
			fatal(state, "Flag Z is NOT set when it is supposed to be");
		}

		break;
	}

	switch(flag_req[1])
	{
	case '-':
		// Check to see if it has changed
		if((REG_F(state) ^ flags_prev) & FLAG_N)
		{
			dump_all_state_invalid_flag(state, opcode, cb, pc_prev, flags_prev);
			fatal(state, "Flag N changed when it wasn't supposed to");
		}

		break;
	case '0':
		// Check to ensure the flag has been cleared
		if(IS_FLAG(state, FLAG_N))
		{
			dump_all_state_invalid_flag(state, opcode, cb, pc_prev, flags_prev);
			fatal(state, "Flag N is set when it is NOT supposed to be");
		}

		break;
	case '1':
		// Check to ensure the flag has been set
		if(!IS_FLAG(state, FLAG_N))
		{
			dump_all_state_invalid_flag(state, opcode, cb, pc_prev, flags_prev);
			fatal(state, "Flag N is NOT set when it is supposed to be");
		}

		break;
	}

	switch(flag_req[2])
	{
	case '-':
		// Check to see if it has changed
		if((REG_F(state) ^ flags_prev) & FLAG_H)
		{
			dump_all_state_invalid_flag(state, opcode, cb, pc_prev, flags_prev);
			fatal(state, "Flag H changed when it wasn't supposed to");
		}

		break;
	case '0':
		// Check to ensure the flag has been cleared
		if(IS_FLAG(state, FLAG_H))
		{
			dump_all_state_invalid_flag(state, opcode, cb, pc_prev, flags_prev);
			fatal(state, "Flag H is set when it is NOT supposed to be");
		}

		break;
	case '1':
		// Check to ensure the flag has been set
		if(!IS_FLAG(state, FLAG_H))
		{
			dump_all_state_invalid_flag(state, opcode, cb, pc_prev, flags_prev);
			fatal(state, "Flag H is NOT set when it is supposed to be");
		}

		break;
	}

	switch(flag_req[3])
	{
	case '-':
		// Check to see if it has changed
		if((REG_F(state) ^ flags_prev) & FLAG_C)
		{
			dump_all_state_invalid_flag(state, opcode, cb, pc_prev, flags_prev);
			fatal(state, "Flag C changed when it wasn't supposed to");
		}

		break;
	case '0':
		// Check to ensure the flag has been cleared
		if(IS_FLAG(state, FLAG_C))
		{
			dump_all_state_invalid_flag(state, opcode, cb, pc_prev, flags_prev);
			fatal(state, "Flag C is set when it is NOT supposed to be");
		}

		break;
	case '1':
		// Check to ensure the flag has been set
		if(!IS_FLAG(state, FLAG_C))
		{
			dump_all_state_invalid_flag(state, opcode, cb, pc_prev, flags_prev);
			fatal(state, "Flag C is NOT set when it is supposed to be");
		}

		break;
	}
#endif /*NDEBUG*/

	return true;
}
