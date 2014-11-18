#include <stdbool.h>	// bool
#include <stdint.h>	// integer types
#include <stdlib.h>	// NULL

#include "memory.h"	// mem_[read|write][8|16]
#include "params.h"	// system_types
#include "print.h"	// fatal
#include "sgherm.h"	// emulator_state, REG_*, etc


#define WAIT_CYCLE(cycles, handler)  handler


/*! Zero Flag */
#define FLAG_Z 0x80
/*! Subtract Flag */
#define FLAG_N 0x40
/*! Half-Carry Flag */
#define FLAG_H 0x20
/*! Carry Flag */
#define FLAG_C 0x10

void dump_flags(emulator_state *state)
{
	debug("flags = %s%s%s%s",
	      (state->flag_reg & FLAG_Z) ? "Z":"z",
	      (state->flag_reg & FLAG_N) ? "N":"n",
	      (state->flag_reg & FLAG_H) ? "H":"h",
	      (state->flag_reg & FLAG_C) ? "C":"c");
}

/*!
 * @brief NOP (0x00)
 * @result Nothing.
 */
void nop(emulator_state *state)
{
	state->pc++;
}

/*!
 * @brief JR n (0x18)
 * @result add n to pc
 */
void jr_imm8(emulator_state *state)
{
	uint8_t to_add = mem_read8(state, ++state->pc);

	state->pc += to_add + 1;
}

/*!
 * @brief JR NZ,n (0x20)
 * @result add n to pc if Z (zero) flag clear
 */
void jr_nz_imm8(emulator_state *state)
{
	uint8_t to_add = mem_read8(state, ++state->pc) + 1;

	state->pc += (state->flag_reg & FLAG_Z) ? 1 : to_add;
}

/*!
 * @brief JR Z,n (0x28)
 * @result add n to pc if Z (zero) flag set
 */
void jr_z_imm8(emulator_state *state)
{
	uint8_t to_add = mem_read8(state, ++state->pc) + 1;

	state->pc += (state->flag_reg & FLAG_Z) ? to_add : 1;
}

/*!
 * @brief LD A,n (0x3E)
 * @result A = n
 */
void ld_a_imm8(emulator_state *state)
{
	*REG_A(state) = mem_read8(state, ++state->pc);
	state->pc++;
}

/*!
 * @brief LD B,A (0x47)
 * @result B = A
 */
void ld_b_a(emulator_state *state)
{
	*REG_B(state) = *REG_A(state);
	state->pc++;
}

/*!
 * @brief LD C,A (0x4F)
 * @result C = A
 */
void ld_c_a(emulator_state *state)
{
	*REG_C(state) = *REG_A(state);
	state->pc++;
}

/*!
 * @brief LD D,A (0x57)
 * @result D = A
 */
void ld_d_a(emulator_state *state)
{
	*REG_D(state) = *REG_A(state);
	state->pc++;
}

/*!
 * @brief LD E,A (0x5F)
 * @result E = A
 */
void ld_e_a(emulator_state *state)
{
	*REG_E(state) = *REG_A(state);
	state->pc++;
}

/*!
 * @brief LD H,A (0x67)
 * @result H = A
 */
void ld_h_a(emulator_state *state)
{
	*REG_H(state) = *REG_A(state);
	state->pc++;
}

/*!
 * @brief LD L,A (0x6F)
 * @result L = A
 */
void ld_l_a(emulator_state *state)
{
	*REG_L(state) = *REG_A(state);
	state->pc++;
}

void xor_common(emulator_state *state, char to_xor)
{
	/* XXX this should be a macro */
	*REG_A(state) ^= to_xor;

	state->flag_reg = 0;
	if(*REG_A(state) == 0) state->flag_reg |= FLAG_Z;

	state->pc++;
}

/*!
 * @brief XOR B (0xA8)
 * @result A ^= B; Z flag set if A is now zero
 */
void xor_b(emulator_state *state)
{
	xor_common(state, *REG_B(state));
}

/*!
 * @brief XOR C (0xA9)
 * @result A ^= C; Z flag set if A is now zero
 */
void xor_c(emulator_state *state)
{
	xor_common(state, *REG_C(state));
}

/*!
 * @brief XOR D (0xAA)
 * @result A ^= D; Z flag set if A is now zero
 */
void xor_d(emulator_state *state)
{
	xor_common(state, *REG_D(state));
}

/*!
 * @brief XOR E (0xAB)
 * @result A ^= E; Z flag set if A is now zero
 */
void xor_e(emulator_state *state)
{
	xor_common(state, *REG_E(state));
}

/*!
 * @brief XOR H (0xAC)
 * @result A ^= H; Z flag set if A is now zero
 */
void xor_h(emulator_state *state)
{
	xor_common(state, *REG_H(state));
}

/*!
 * @brief XOR L (0xAD)
 * @result A ^= H; Z flag set if A is now zero
 */
void xor_l(emulator_state *state)
{
	xor_common(state, *REG_L(state));
}

/*!
 * @brief XOR A (0xAF)
 * @result A = 0; Z flag set
 */
void xor_a(emulator_state *state)
{
	*REG_A(state) = 0;
	state->flag_reg = FLAG_Z;
	state->pc++;
}

/*!
 * @brief JP nn (0xC3)
 * @result pc is set to 16-bit immediate value (LSB, MSB)
 */
void jp_imm16(emulator_state *state)
{
	uint8_t lsb = mem_read8(state, ++state->pc);
	uint8_t msb = mem_read8(state, ++state->pc);

	state->pc = (msb<<8 | lsb);
}

enum cb_regs {
	CB_REG_B = 0, CB_REG_C, CB_REG_D, CB_REG_E, CB_REG_H, CB_REG_L,
	CB_REG_HL, CB_REG_A
};

enum cb_ops {
	CB_OP_BIT = 1, CB_OP_RES = 2, CB_OP_SET = 3
};

/*!
 * @brief CB ..
 * @note this is just a dispatch function for SWAP/BIT/etc
 */
void cb_dispatch(emulator_state *state)
{
	uint8_t opcode = mem_read8(state, ++state->pc);

	if(opcode >= 0x40)
	{
		uint8_t bit_number = (opcode & 0x38) >> 3;
		enum cb_regs reg = (opcode & 0x7);
		enum cb_ops op = (opcode & 0xC0) >> 6;
		uint8_t *write_to;
		uint8_t maybe_temp;

		switch(reg)
		{
		case CB_REG_B:
			write_to = REG_B(state); break;
		case CB_REG_C:
			write_to = REG_C(state); break;
		case CB_REG_D:
			write_to = REG_D(state); break;
		case CB_REG_E:
			write_to = REG_E(state); break;
		case CB_REG_H:
			write_to = REG_H(state); break;
		case CB_REG_L:
			write_to = REG_L(state); break;
		case CB_REG_HL:
			write_to = &maybe_temp; break;
		case CB_REG_A:
			write_to = REG_A(state); break;
		}

		uint8_t val = (1 << bit_number);

		if(reg == CB_REG_HL)
		{
			maybe_temp = mem_read8(state, state->hl);
		}

		switch(op)
		{
		case CB_OP_RES:
			// reset bit <bit_number> of register <reg>
			//debug("reset bit %d of reg %d", bit_number, reg);
			*write_to &= !val; break;
		case CB_OP_SET:
			// set bit <bit_number> of register <reg>
			//debug("set bit %d of reg %d", bit_number, reg);
			*write_to |= val;  break;
		case CB_OP_BIT:
			// test bit <bit_number> of register <reg>
			//debug("test bit %d of reg %d", bit_number, reg);
			dump_flags(state);
			state->flag_reg |= (*write_to & val) ? FLAG_Z : !FLAG_Z;
			state->flag_reg |= FLAG_H;
			state->flag_reg &= !FLAG_N;
			dump_flags(state);
			break;
		}

		if(reg == CB_REG_HL)
		{
			mem_write8(state, state->hl, maybe_temp);
		}
	} else {
		debug("IGNORING CB op %02X", opcode);
	}

	state->pc++;
}

/*!
 * @brief CALL nn (0xCD)
 * @result next pc stored in stack; jump to nn
 */
void call_imm16(emulator_state *state)
{
	uint8_t lsb = mem_read8(state, ++state->pc);
	uint8_t msb = mem_read8(state, ++state->pc);

	state->sp -= 2;
	mem_write16(state, state->sp, ++state->pc);

	state->pc = (msb<<8 | lsb);
}

/*!
 * @brief LDH n,A (0xE0) - write A to 0xff00+n
 * @result the I/O register n will contain the value of A
 */
void ldh_imm8_a(emulator_state *state)
{
	uint16_t write = mem_read8(state, ++state->pc);
	write += 0xFF00;

	mem_write8(state, write, *REG_A(state));

	state->pc++;
}

/*!
 * @brief LD (nn),A (0xEA) - write A to *nn
 * @result the memory at address nn will contain the value of A
 */
void ld_d16_a(emulator_state *state)
{
	uint8_t lsb = mem_read8(state, ++state->pc);
	uint8_t msb = mem_read8(state, ++state->pc);

	uint16_t loc = (msb<<8) | lsb;

	mem_write8(state, loc, *REG_A(state));

	state->pc++;
}

/*!
 * @brief LDH A,nn (0xF0) - read 0xff00+n to A
 * @result A will contain the value of the I/O register n
 */
void ldh_a_imm8(emulator_state *state)
{
	uint8_t loc = mem_read8(state, ++state->pc);
	*REG_A(state) = mem_read8(state, 0xFF00 + loc);

	state->pc++;
}

/*!
 * @brief DI (0xF3) - disable interrupts
 * @result interrupts will be disabled the instruction AFTER this one
 */
void di(emulator_state *state)
{
	state->toggle_int_on_next = true;

	state->pc++;
}

/*!
 * @brief EI (0xFB) - enable interrupts
 * @result interrupts will be enabled the instruction AFTER this one
 */
void ei(emulator_state *state)
{
	state->toggle_int_on_next = true;

	state->pc++;
}

/*!
 * @brief CP n (0xFE) - compare A with 8-bit immediate value
 * @result flags register modified based on result
 */
void cp_imm8(emulator_state *state)
{
	uint8_t cmp;

	cmp = mem_read8(state, ++state->pc);
	debug("flags = %s%s%s%s; cmp = %d; A = %d",
	       (state->flag_reg & FLAG_Z) ? "Z":"z",
	       (state->flag_reg & FLAG_N) ? "N":"n",
	       (state->flag_reg & FLAG_H) ? "H":"h",
	       (state->flag_reg & FLAG_C) ? "C":"c", cmp, *REG_A(state));
	state->flag_reg |= FLAG_N;
	state->flag_reg &= ~FLAG_H | ~FLAG_C;
	if(*REG_A(state) == cmp)
	{
		state->flag_reg |= FLAG_Z;
	} else {
		state->flag_reg &= ~FLAG_Z;
		if(*REG_A(state) < cmp)
		{
			state->flag_reg |= FLAG_C;
		} else {
			state->flag_reg |= FLAG_H;
		}
	}
	dump_flags(state);

	state->pc++;
}

typedef void (*opcode_t)(emulator_state *state);

opcode_t handlers[0x100] = {
	/* 0x00 */ nop, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0x08 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0x10 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0x18 */ jr_imm8, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0x20 */ jr_nz_imm8, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0x28 */ jr_z_imm8, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0x30 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0x38 */ NULL, NULL, NULL, NULL, NULL, NULL, ld_a_imm8, NULL,
	/* 0x40 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, ld_b_a,
	/* 0x48 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, ld_c_a,
	/* 0x50 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, ld_d_a,
	/* 0x58 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, ld_e_a,
	/* 0x60 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, ld_h_a,
	/* 0x68 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, ld_l_a,
	/* 0x70 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0x78 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0x80 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0x88 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0x90 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0x98 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0xA0 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0xA8 */ xor_b, xor_c, xor_d, xor_e, xor_h, xor_l, NULL, xor_a,
	/* 0xB0 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0xB8 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0xC0 */ NULL, NULL, NULL, jp_imm16, NULL, NULL, NULL, NULL,
	/* 0xC8 */ NULL, NULL, NULL, cb_dispatch, NULL, call_imm16, NULL, NULL,
	/* 0xD0 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0xD8 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0xE0 */ ldh_imm8_a, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0xE8 */ NULL, NULL, ld_d16_a, NULL, NULL, NULL, NULL, NULL,
	/* 0xF0 */ ldh_a_imm8, NULL, NULL, di, NULL, NULL, NULL, NULL,
	/* 0xF8 */ NULL, NULL, NULL, ei, NULL, NULL, cp_imm8, NULL
};

char cycles[0x100] = {
	/* 0x00 */ 4, 12, 8, 8, 4, 4, 8, 4,
	/* 0x08 */ 20, 8, 8, 8, 4, 4, 8, 4,
	/* 0x10 */ 4, 12, 8, 8, 4, 4, 8, 4,
	/* 0x18 */ 12, 8, 8, 8, 4, 4, 8, 4,
	/* 0x20 */ 8, 12, 8, 8, 4, 4, 8, 4,
	/* 0x28 */ 8, 8, 8, 8, 4, 4, 8, 4,
	/* 0x30 */ 8, 12, 8, 8, 12, 12, 4, 4,
	/* 0x38 */ 8, 8, 8, 8, 4, 4, 8, 4,
	/* 0x40 */ 4, 4, 4, 4, 4, 4, 8, 4,
	/* 0x48 */ 4, 4, 4, 4, 4, 4, 8, 4,
	/* 0x50 */ 4, 4, 4, 4, 4, 4, 8, 4,
	/* 0x58 */ 4, 4, 4, 4, 4, 4, 8, 4,
	/* 0x60 */ 4, 4, 4, 4, 4, 4, 8, 4,
	/* 0x68 */ 4, 4, 4, 4, 4, 4, 8, 4,
	/* 0x70 */ 8, 8, 8, 8, 8, 8, 4, 8,
	/* 0x78 */ 4, 4, 4, 4, 4, 4, 8, 4,
	/* 0x80 */ 4, 4, 4, 4, 4, 4, 8, 4,
	/* 0x88 */ 4, 4, 4, 4, 4, 4, 8, 4,
	/* 0x90 */ 4, 4, 4, 4, 4, 4, 8, 4,
	/* 0x98 */ 4, 4, 4, 4, 4, 4, 8, 4,
	/* 0xA0 */ 4, 4, 4, 4, 4, 4, 8, 4,
	/* 0xA8 */ 4, 4, 4, 4, 4, 4, 8, 4,
	/* 0xB0 */ 4, 4, 4, 4, 4, 4, 8, 4,
	/* 0xB8 */ 4, 4, 4, 4, 4, 4, 8, 4,
	/* 0xC0 */ 8, 12, 12, 16, 12, 16, 8, 16,
	/* 0xC8 */ 8, 16, 12, 4, 12, 24, 8, 16,
	/* 0xD0 */ 8, 12, 12, 0000, 12, 16, 8, 16,
	/* 0xD8 */ 8, 16, 12, 0000, 12, 0000, 8, 16,
	/* 0xE0 */ 12, 12, 8, 0000, 0000, 16, 8, 16,
	/* 0xE8 */ 16, 4, 16, 0000, 0000, 0000, 8, 16,
	/* 0xF0 */ 12, 12, 8, 4, 0000, 16, 8, 16,
	/* 0xF8 */ 12, 8, 16, 4, 0000, 0000, 8, 16
};


/*! boot up */
void init_ctl(emulator_state *state, char type)
{
	state->pc = 0x0100;
	switch(type)
	{
	case SYSTEM_SGB:
		debug("Super Game Boy emulation");
		*REG_A(state) = 0x01;
		break;
	case SYSTEM_GB:
	default:
		debug("original Game Boy emulation");
		*REG_A(state) = 0x01;
		break;
	case SYSTEM_GBC:
		debug("Game Boy Color emulation");
		*REG_A(state) = 0x11;
		break;
	case SYSTEM_GBP:
		debug("Game Boy Portable emulation");
		*REG_A(state) = 0xFF;
		break;
	}
	*REG_F(state) = 0xB0;
	*REG_B(state) = 0x00;
	*REG_C(state) = 0x13;
	*REG_D(state) = 0x00;
	*REG_E(state) = 0xD8;
	*REG_H(state) = 0x01;
	*REG_L(state) = 0x4D;
	state->sp = 0xFFFE;
}


/*! the emulated CU for the 'z80-ish' CPU */
bool execute(emulator_state *state)
{
	unsigned char opcode = mem_read8(state, state->pc);
	opcode_t handler = handlers[opcode];
	bool toggle = state->toggle_int_on_next;

	if(handler == NULL)
	{
		fatal("invalid opcode %02X at %04X", opcode, state->pc);
	}

	WAIT_CYCLE(cycles[opcode], handler(state));

	if(toggle)
	{
		state->toggle_int_on_next = false;
		state->interrupts = !state->interrupts;
	}

	return true;
}
