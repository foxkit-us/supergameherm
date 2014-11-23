#include "sgherm.h"	// emu_state, etc.
#include "ctl_unit.h"	// prototypes, constants, etc.
#include "print.h"	// fatal
#include "debug.h"	// lookup_mnemonic

#include <stdbool.h>	// bool
#include <stdint.h>	// integer types
#include <stdlib.h>	// NULL


uint8_t int_flag_read(emu_state *restrict state, uint16_t location unused)
{
	return state->int_state.pending;
}

void int_flag_write(emu_state *restrict state, uint16_t location unused, uint8_t data)
{
	uint8_t mask = state->int_state.mask;

	if(data > state->int_state.pending)
	{
		// Recompute jmp address
		if((data & INT_VBLANK) && !(mask & INT_VBLANK))
		{
			state->int_state.next_jmp = INT_ID_VBLANK;
		}
		else if((data & INT_LCD_STAT) && !(mask & INT_LCD_STAT))
		{
			state->int_state.next_jmp = INT_ID_LCD_STAT;
		}
		else if((data & INT_TIMER) && !(mask & INT_TIMER))
		{
			state->int_state.next_jmp = INT_ID_TIMER;
		}
		else if((data & INT_SERIAL) && !(mask & INT_SERIAL))
		{
			state->int_state.next_jmp = INT_ID_SERIAL;
		}
		else if((data & INT_JOYPAD) && !(mask & INT_JOYPAD))
		{
			state->int_state.next_jmp = INT_ID_JOYPAD;
		}
		else
		{
			state->int_state.next_jmp = INT_ID_NONE;
		}
	}

	printf("[triggered] Next jump point: %04X\n", state->int_state.next_jmp);

	state->int_state.pending = data;
}

void int_mask_flag_write(emu_state *restrict state, uint8_t data)
{
	uint8_t mask = state->int_state.pending & data;

	if(!mask)
	{
		// Masked
		state->int_state.next_jmp = INT_ID_NONE;
	}
	else
	{
		// Recompute jmp address
		if(mask & INT_VBLANK)
		{
			state->int_state.next_jmp = INT_ID_VBLANK;
		}
		else if(mask & INT_LCD_STAT)
		{
			state->int_state.next_jmp = INT_ID_LCD_STAT;
		}
		else if(mask & INT_TIMER)
		{
			state->int_state.next_jmp = INT_ID_TIMER;
		}
		else if(mask & INT_SERIAL)
		{
			state->int_state.next_jmp = INT_ID_SERIAL;
		}
		else if(mask & INT_JOYPAD)
		{
			state->int_state.next_jmp = INT_ID_JOYPAD;
		}
		else
		{
			state->int_state.next_jmp = INT_ID_NONE;
		}
	}

	printf("[masked] Next jump point: %04X\n", state->int_state.next_jmp);

	state->int_state.mask = data;
}

#include "instr_alu_arith.c"
#include "instr_alu_logic.c"
#include "instr_branch.c"
#include "instr_intr.c"
#include "instr_ld.c"
#include "instr_misc.c"
#include "instr_stack.c"


static const opcode_t handlers[0x100] =
{
	/* 0x00 */ nop, ld_bc_imm16, ld_bc_a, inc_bc, inc_b, dec_b, ld_b_imm8, rlca,
	/* 0x08 */ not_impl, add_hl_bc, ld_a_bc, dec_bc, inc_c, dec_c, ld_c_imm8, rrca,
	/* 0x10 */ stop, ld_de_imm16, ld_de_a, inc_de, inc_d, dec_d, ld_d_imm8, rla,
	/* 0x18 */ jr_imm8, add_hl_de, ld_a_de, dec_de, inc_e, dec_e, ld_e_imm8, rra,
	/* 0x20 */ jr_nz_imm8, ld_hl_imm16, ldi_hl_a, inc_hl, inc_h, dec_h, ld_h_imm8, daa,
	/* 0x28 */ jr_z_imm8, add_hl_hl, ldi_a_hl, dec_hl, inc_l, dec_l, ld_l_imm8, cpl,
	/* 0x30 */ jr_nc_imm8, ld_sp_imm16, ldd_hl_a, inc_sp, inc_hl, dec_hl, ld_hl_imm8, scf,
	/* 0x38 */ jr_c_imm8, add_hl_sp, ldd_a_hl, dec_sp, inc_a, dec_a, ld_a_imm8, ccf,
	/* 0x40 */ ld_b_b, ld_b_c, ld_b_d, ld_b_e, ld_b_h, ld_b_l, ld_b_hl, ld_b_a,
	/* 0x48 */ ld_c_b, ld_c_c, ld_c_d, ld_c_e, ld_c_h, ld_c_l, ld_c_hl, ld_c_a,
	/* 0x50 */ ld_d_b, ld_d_c, ld_d_d, ld_d_e, ld_d_h, ld_d_l, ld_d_hl, ld_d_a,
	/* 0x58 */ ld_e_b, ld_e_c, ld_e_d, ld_e_e, ld_e_h, ld_e_l, ld_e_hl, ld_e_a,
	/* 0x60 */ ld_h_b, ld_h_c, ld_h_d, ld_h_e, ld_h_h, ld_h_l, ld_h_hl, ld_h_a,
	/* 0x68 */ ld_l_b, ld_l_c, ld_l_d, ld_l_e, ld_l_h, ld_l_l, ld_l_hl, ld_l_a,
	/* 0x70 */ ld_hl_b, ld_hl_c, ld_hl_d, ld_hl_e, ld_hl_h, ld_hl_l, halt, ld_hl_a,
	/* 0x78 */ ld_a_b, ld_a_c, ld_a_d, ld_a_e, ld_a_h, ld_a_l, ld_a_hl, ld_a_a,
	/* 0x80 */ add_b, add_c, add_d, add_e, add_h, add_l, add_hl, add_a,
	/* 0x88 */ adc_b, adc_c, adc_d, adc_e, adc_h, adc_h, adc_hl, adc_a,
	/* 0x90 */ sub_b, sub_c, sub_d, sub_e, sub_h, sub_l, sub_hl, sub_a,
	/* 0x98 */ sbc_b, sbc_c, sbc_d, sbc_e, sbc_h, sbc_l, sbc_hl, sbc_a,
	/* 0xA0 */ and_b, and_c, and_d, and_e, and_h, and_l, and_hl, and_a,
	/* 0xA8 */ xor_b, xor_c, xor_d, xor_e, xor_h, xor_l, xor_hl, xor_a,
	/* 0xB0 */ or_b, or_c, or_d, or_e, or_h, or_l, or_hl, or_a,
	/* 0xB8 */ cp_b, cp_c, cp_d, cp_e, cp_h, cp_l, cp_hl, cp_a,
	/* 0xC0 */ retnz, pop_bc, jp_nz_imm16, jp_imm16, call_nz_imm16, push_bc, add_imm8, reset_common,
	/* 0xC8 */ retz, ret, jp_z_imm16, cb_dispatch, call_z_imm16, call_imm16, adc_imm8, reset_common,
	/* 0xD0 */ retnc, pop_de, jp_nc_imm16, invalid, call_nc_imm16, push_de, sub_imm8, reset_common,
	/* 0xD8 */ retc, reti, jp_c_imm16, invalid, call_c_imm16, invalid, sbc_imm8, reset_common,
	/* 0xE0 */ ldh_imm8_a, pop_hl, ld_ff00_c_a, invalid, invalid, push_hl, and_imm8, reset_common,
	/* 0xE8 */ not_impl, jp_hl, ld_d16_a, invalid, invalid, invalid, xor_imm8, reset_common,
	/* 0xF0 */ ldh_a_imm8, pop_af, ld_a_ff00_c, di, invalid, push_af, or_imm8, reset_common,
	/* 0xF8 */ ld_hl_sp_imm8, ld_sp_hl, ld_a_d16, ei, invalid, invalid, cp_imm8, reset_common
};


/*! boot up */
void init_ctl(emu_state *restrict state, system_types type)
{
	REG_PC(state) = 0x0100;
	switch(type)
	{
	case SYSTEM_SGB:
		debug("Super Game Boy emulation");
		REG_A(state) = 0x01;
		break;
	case SYSTEM_GBC:
		debug("Game Boy Color emulation");
		REG_A(state) = 0x11;
		break;
	case SYSTEM_GBP:
		debug("Game Boy Portable emulation");
		REG_A(state) = 0xFF;
		break;
	case SYSTEM_GB:
	default:
		debug("original Game Boy emulation");
		REG_A(state) = 0x01;
		break;
	}
	REG_F(state) = 0xB0;
	REG_B(state) = 0x00;
	REG_C(state) = 0x13;
	REG_D(state) = 0x00;
	REG_E(state) = 0xD8;
	REG_H(state) = 0x01;
	REG_L(state) = 0x4D;
	REG_SP(state) = 0xFFFE;
}


/*! Do a call to an interrupt handler */
static inline void call_interrupt(emu_state *restrict state, uint8_t interrupts)
{
	interrupt_list jmp_offset;

	// Clear interrupts
	mem_write8(state, 0xFF0F, 0);

	// Interrupts are locked out before handling
	state->int_state.enabled = false;

	if(interrupts & INT_VBLANK)
	{
		jmp_offset = INT_ID_VBLANK;
	}
	else if(interrupts & INT_LCD_STAT)
	{
		jmp_offset = INT_ID_LCD_STAT;
	}
	else if(interrupts & INT_TIMER)
	{
		jmp_offset = INT_ID_TIMER;
	}
	else if(interrupts & INT_SERIAL)
	{
		jmp_offset = INT_ID_SERIAL;
	}
	else if(interrupts & INT_JOYPAD)
	{
		jmp_offset = INT_ID_JOYPAD;
	}
	else
	{
		fatal("Unknown interrupt caught (mask: %X)", interrupts);
		return;
	}

	// I don't trust my interrupt write code yet, so...
	if(state->int_state.next_jmp != jmp_offset)
	{
		fatal("jmp_offset/next_jmp mismatch: %4X vs %4X",
				state->int_state.next_jmp, jmp_offset);
		return;
	}

	// Push pc to the stack
	REG_SP(state) -= 2;
	mem_write16(state, REG_SP(state), REG_PC(state));

	// Jump!
	REG_PC(state) = jmp_offset;

	// Reset these states
	state->halt = state->stop = false;

	// XXX is this right?
	state->wait = 24;

	debug("Interrupt jumping to %04X", REG_PC(state));
}


/*! the emulated CU for the 'z80-ish' CPU */
bool execute(emu_state *restrict state)
{
	uint8_t opcode;
	opcode_t handler;

	if(--state->wait)
	{
		return true;
	}

	if(unlikely(state->dma_membar_wait))
	{
		state->dma_membar_wait--;
	}

	// Check for interrupts
	if(state->int_state.enabled)
	{
		uint8_t interrupts = mem_read8(state, 0xFF0F) & mem_read8(state, 0xFFFF);
		if(interrupts)
		{
			call_interrupt(state, interrupts);
		}
	}

	if(state->halt || state->stop)
	{
		// Waiting for an interrupt
		return true;
	}

	opcode = mem_read8(state, REG_PC(state));
	handler = handlers[opcode];
	//fprintf(stderr, "pc=%04X\n", REG_PC(state));
	handler(state);

	return true;
}
