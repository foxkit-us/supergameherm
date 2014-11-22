#include "config.h"	// macros

#include <stdbool.h>	// bool
#include <stdint.h>	// integer types
#include <stdlib.h>	// NULL

#include "sgherm.h"	// emu_state, REG_*, etc
#include "ctl_unit.h"	// constants
#include "memory.h"	// mem_[read|write][8|16]
#include "params.h"	// system_types
#include "print.h"	// fatal
#include "debug.h"	// lookup_mnemonic

uint8_t int_flag_read(emu_state *restrict state, uint16_t location)
{
	return state->memory[location];
}

void int_flag_write(emu_state *restrict state, uint16_t location, uint8_t data)
{
	/* only allow setting of the first five bits. */
	state->memory[location] = data & 0x1F;
}

/*!
 * @brief Unimplemented opcode (multiple values)
 * @result Terminates emulator
 * @note This function will go away when all opcodes are implemented
 */
static inline void not_impl(emu_state *restrict state unused)
{
	uint8_t opcode = mem_read8(state, state->registers.pc);
	fatal("Unimplemented opcode %2X at %4X (nmemonic %s)", opcode,
			state->registers.pc, lookup_mnemonic(opcode));
}

/*!
 * @brief Invalid opcode (multiple values)
 * @result Terminates emulator
 */
static inline void invalid(emu_state *restrict state unused)
{
	fatal("Invalid opcode");
}

/*!
 * @brief NOP (0x00)
 * @result Nothing.
 */
static inline void nop(emu_state *restrict state)
{
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief LD BC,nn (0x01)
 * @result BC = nn
 */
static inline void ld_bc_imm16(emu_state *restrict state)
{
	uint8_t lsb = mem_read8(state, ++state->registers.pc);
	uint8_t msb = mem_read8(state, ++state->registers.pc);

	state->registers.bc = (msb<<8)|lsb;

	state->registers.pc++;

	state->wait = 12;
}

/*!
 * @brief LD (BC),A (0x02)
 * @result contents of memory at BC = A
 */
static inline void ld_bc_a(emu_state *restrict state)
{
	mem_write8(state, state->registers.bc, *(state->registers.a));

	state->registers.pc++;

	state->wait = 8;
}

/*!
 * @brief INC BC (0x03)
 * @result 1 is added to BC (possibly wrapping)
 */
static inline void inc_bc(emu_state *restrict state)
{
	state->registers.bc++;
	state->registers.pc++;

	state->wait = 8;
}

static inline void inc_r8(emu_state *restrict state, uint8_t *reg)
{
	//uint8_t old = *reg;

	if(*reg ^ 0x0F)
	{
		*(state->registers.f) &= ~FLAG_H;
	}
	else
	{
		*(state->registers.f) |= FLAG_H;
	}

	*reg += 1;

	if(!(*reg))
	{
		*(state->registers.f) |= FLAG_Z;
	}

	*(state->registers.f) &= ~FLAG_N;

	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief INC B (0x04)
 * @result 1 is added to B; Z if B is now zero, H if bit 3 overflow
 */
static inline void inc_b(emu_state *restrict state)
{
	inc_r8(state, state->registers.b);
}

static inline void dec_r8(emu_state *restrict state, uint8_t *reg)
{
	//uint8_t old = *reg;

	*(state->registers.f) = FLAG_N;

	if(*reg & 0x0F)
	{
		*(state->registers.f) &= ~FLAG_H;
	}
	else
	{
		*(state->registers.f) |= FLAG_H;
	}

	*reg -= 1;

	if(!(*reg))
	{
		*(state->registers.f) |= FLAG_Z;
	}
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief DEC B (0x05)
 * @result 1 is subtracted from B; Z if B is now zero, H if bit 4 underflow
 */
static inline void dec_b(emu_state *restrict state)
{
	dec_r8(state, state->registers.b);
}

/*!
 * @brief LD B,n (0x06)
 * @result B = n
 */
static inline void ld_b_imm8(emu_state *restrict state)
{
	*(state->registers.b) = mem_read8(state, ++state->registers.pc);
	state->registers.pc++;

	state->wait = 8;
}

static inline void add_to_hl(emu_state *restrict state, uint16_t to_add)
{
	if((uint32_t)(state->registers.hl + to_add) > 0xFFFF)
	{
		*(state->registers.f) |= FLAG_C;
	}
	else
	{
		*(state->registers.f) &= ~FLAG_C;
	}

	if((state->registers.hl & 0xF) + (to_add & 0xF) > 0xF)
	{
		*(state->registers.f) |= FLAG_H;
	}
	else
	{
		*(state->registers.f) &= ~FLAG_H;
	}

	*(state->registers.f) &= ~FLAG_N;

	state->registers.hl += to_add;

	state->registers.pc++;

	state->wait = 12;
}

/*!
 * @brief ADD HL,BC (0x09)
 * @result HL += BC; N flag reset, H if carry from bit 11, C if overflow
 */
static inline void add_hl_bc(emu_state *restrict state)
{
	add_to_hl(state, state->registers.bc);
}

/*!
 * @brief LD A,(BC) (0x0A)
 * @result A = contents of memory at BC
 */
static inline void ld_a_bc(emu_state *restrict state)
{
	*(state->registers.a) = mem_read8(state, state->registers.bc);
	state->registers.pc++;

	state->wait = 8;
}

/*!
 * @brief DEC BC (0x0B)
 * @result 1 is subtracted from BC (possibly wrapping)
 */
static inline void dec_bc(emu_state *restrict state)
{
	state->registers.bc--;
	state->registers.pc++;

	state->wait = 8;
}

/*!
 * @brief INC C (0x0C)
 * @result 1 is added to C; Z if C is now zero, H if bit 3 overflow
 */
static inline void inc_c(emu_state *restrict state)
{
	inc_r8(state, state->registers.c);
}

/*!
 * @brief DEC C (0x0D)
 * @result 1 is subtracted from C; Z if C is now zero, H if bit 4 underflow
 */
static inline void dec_c(emu_state *restrict state)
{
	dec_r8(state, state->registers.c);
}

/*!
 * @brief LD C,n (0x0E)
 * @result C = n
 */
static inline void ld_c_imm8(emu_state *restrict state)
{
	*(state->registers.c) = mem_read8(state, ++state->registers.pc);
	state->registers.pc++;

	state->wait = 8;
}

/*!
 * @brief LD DE,nn (0x11)
 * @result DE = nn
 */
static inline void ld_de_imm16(emu_state *restrict state)
{
	uint8_t lsb = mem_read8(state, ++state->registers.pc);
	uint8_t msb = mem_read8(state, ++state->registers.pc);

	state->registers.de = (msb<<8)|lsb;

	state->registers.pc++;

	state->wait = 12;
}

/*!
 * @brief LD (DE),A (0x12)
 * @result contents of memory at DE = A
 */
static inline void ld_de_a(emu_state *restrict state)
{
	mem_write8(state, state->registers.de, *(state->registers.a));

	state->registers.pc++;

	state->wait = 8;
}

/*!
 * @brief INC DE (0x13)
 * @result 1 is added to DE (possibly wrapping)
 */
static inline void inc_de(emu_state *restrict state)
{
	state->registers.de++;
	state->registers.pc++;

	state->wait = 8;
}

/*!
 * @brief INC D (0x14)
 * @result 1 is added to D; Z if D is now zero, H if bit 3 overflow
 */
static inline void inc_d(emu_state *restrict state)
{
	inc_r8(state, state->registers.d);
}

/*!
 * @brief DEC D (0x15)
 * @result 1 is subtracted from D; Z if D is now zero, H if bit 4 underflow
 */
static inline void dec_d(emu_state *restrict state)
{
	dec_r8(state, state->registers.d);
}

/*!
 * @brief LD D,n (0x16)
 * @result D = n
 */
static inline void ld_d_imm8(emu_state *restrict state)
{
	*(state->registers.d) = mem_read8(state, ++state->registers.pc);
	state->registers.pc++;

	state->wait = 8;
}

/*!
 * @brief JR n (0x18)
 * @result add n to pc
 */
static inline void jr_imm8(emu_state *restrict state)
{
	int8_t to_add = mem_read8(state, ++state->registers.pc);

	state->registers.pc += to_add + 1;

	state->wait = 12;
}

/*!
 * @brief ADD HL,DE (0x19)
 * @result HL += DE; N flag reset, H if carry from bit 11, C if overflow
 */
static inline void add_hl_de(emu_state *restrict state)
{
	add_to_hl(state, state->registers.de);
}

/*!
 * @brief LD A,(DE) (0x1A)
 * @result A = contents of memory at (DE)
 */
static inline void ld_a_de(emu_state *restrict state)
{
	*(state->registers.a) = mem_read8(state, state->registers.de);

	state->registers.pc++;

	state->wait = 8;
}

/*!
 * @brief DEC DE (0x1B)
 * @result 1 is subtracted from DE (possibly wrapping)
 */
static inline void dec_de(emu_state *restrict state)
{
	state->registers.de--;
	state->registers.pc++;

	state->wait = 8;
}

/*!
 * @brief INC E (0x1C)
 * @result 1 is added to E; Z if E is now zero, H if bit 3 overflow
 */
static inline void inc_e(emu_state *restrict state)
{
	inc_r8(state, state->registers.e);
}

/*!
 * @brief DEC E (0x1D)
 * @result 1 is subtracted from E; Z if E is now zero, H if bit 4 underflow
 */
static inline void dec_e(emu_state *restrict state)
{
	dec_r8(state, state->registers.e);
}

/*!
 * @brief LD E,n (0x1E)
 * @result E = n
 */
static inline void ld_e_imm8(emu_state *restrict state)
{
	*(state->registers.e) = mem_read8(state, ++state->registers.pc);
	state->registers.pc++;

	state->wait = 8;
}

/*!
 * @brief JR NZ,n (0x20)
 * @result add n to pc if Z (zero) flag clear
 */
static inline void jr_nz_imm8(emu_state *restrict state)
{
	int8_t to_add = mem_read8(state, ++state->registers.pc) + 1;

	state->registers.pc += (*(state->registers.f) & FLAG_Z) ? 1 : to_add;

	state->wait = 8;
}

/*!
 * @brief LD HL,nn (0x21)
 * @result HL = nn
 */
static inline void ld_hl_imm16(emu_state *restrict state)
{
	uint8_t lsb = mem_read8(state, ++state->registers.pc);
	uint8_t msb = mem_read8(state, ++state->registers.pc);

	state->registers.hl = (msb<<8)|lsb;

	state->registers.pc++;

	state->wait = 12;
}

/*!
 * @brief LD (HL+),A (0x22)
 * @result contents of memory at HL = A; HL incremented 1
 */
static inline void ldi_hl_a(emu_state *restrict state)
{
	mem_write8(state, state->registers.hl++, *(state->registers.a));
	state->registers.pc++;

	state->wait = 8;
}

/*!
* @brief INC HL (0x23)
* @result 1 is added to HL (possibly wrapping)
*/
static inline void inc_hl(emu_state *restrict state)
{
	state->registers.hl++;
	state->registers.pc++;

	state->wait = 8;
}

/*!
 * @brief INC H (0x24)
 * @result 1 is added to H; Z if H is now zero, H if bit 3 overflow
 */
static inline void inc_h(emu_state *restrict state)
{
	inc_r8(state, state->registers.h);
}

/*!
 * @brief DEC H (0x25)
 * @result 1 is subtracted from H; Z if H is now zero, H if bit 4 underflow
 */
static inline void dec_h(emu_state *restrict state)
{
	dec_r8(state, state->registers.h);
}

/*!
 * @brief LD H,n (0x26)
 * @result H = n
 */
static inline void ld_h_imm8(emu_state *restrict state)
{
	*(state->registers.h) = mem_read8(state, ++state->registers.pc);
	state->registers.pc++;

	state->wait = 8;
}

/*!
 * @brief JR Z,n (0x28)
 * @result add n to pc if Z (zero) flag set
 */
static inline void jr_z_imm8(emu_state *restrict state)
{
	int8_t to_add = mem_read8(state, ++state->registers.pc) + 1;

	state->registers.pc += (*(state->registers.f) & FLAG_Z) ? to_add : 1;

	state->wait = 8;
}

/*!
 * @brief ADD HL,HL (0x29)
 * @result HL += HL; N flag reset, H if carry from bit 11, C if overflow
 */
static inline void add_hl_hl(emu_state *restrict state)
{
	add_to_hl(state, state->registers.hl);
}

/*!
 * @brief LD A,(HL+) (0x2A)
 * @result A = contents of memory at HL; HL incremented 1
 */
static inline void ldi_a_hl(emu_state *restrict state)
{
	*(state->registers.a) = mem_read8(state, state->registers.hl++);
	state->registers.pc++;

	state->wait = 8;
}

/*!
 * @brief DEC HL (0x2B)
 * @result 1 is subtracted from HL (possibly wrapping)
 */
static inline void dec_hl(emu_state *restrict state)
{
	state->registers.hl--;
	state->registers.pc++;

	state->wait = 8;
}

/*!
 * @brief INC L (0x2C)
 * @result 1 is added to L; Z if L is now zero, H if bit 3 overflow
 */
static inline void inc_l(emu_state *restrict state)
{
	inc_r8(state, state->registers.l);
}

/*!
 * @brief DEC L (0x2D)
 * @result 1 is subtracted from L; Z if L is now zero, H if bit 4 underflow
 */
static inline void dec_l(emu_state *restrict state)
{
	dec_r8(state, state->registers.l);
}

/*!
 * @brief LD L,n (0x2E)
 * @result L = n
 */
static inline void ld_l_imm8(emu_state *restrict state)
{
	*(state->registers.l) = mem_read8(state, ++state->registers.pc);
	state->registers.pc++;

	state->wait = 8;
}

/*!
 * @brief CPL (0x2F)
 * @result all bits of A are negated
 */
static inline void cpl(emu_state *restrict state)
{
	*(state->registers.a) ^= ~(*(state->registers.a));
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief JR NC,n (0x30)
 * @result add n to pc if C (carry) flag clear
 */
static inline void jr_nc_imm8(emu_state *restrict state)
{
	int8_t to_add = mem_read8(state, ++state->registers.pc) + 1;

	state->registers.pc += (*(state->registers.f) & FLAG_C) ? 1 : to_add;

	state->wait = 8;
}

/*!
 * @brief LD SP,nn (0x31)
 * @result SP = nn
 */
static inline void ld_sp_imm16(emu_state *restrict state)
{
	uint8_t lsb = mem_read8(state, ++state->registers.pc);
	uint8_t msb = mem_read8(state, ++state->registers.pc);

	state->registers.sp = (msb<<8)|lsb;

	state->registers.pc++;

	state->wait = 12;
}

/*!
 * @brief LD (HL-),A (0x32)
 * @result contents of memory at HL = A; HL decremented 1
 */
static inline void ldd_hl_a(emu_state *restrict state)
{
	mem_write8(state, state->registers.hl--, *(state->registers.a));
	state->registers.pc++;

	state->wait = 8;
}

/*!
 * @brief INC SP (0x33)
 * @result 1 is added to SP (possibly wrapping)
 */
static inline void inc_sp(emu_state *restrict state)
{
	state->registers.sp++;
	state->registers.pc++;

	state->wait = 8;
}

/*!
 * @brief LD (HL),n (0x36)
 * @result contents of memory at HL = n
 */
static inline void ld_hl_imm8(emu_state *restrict state)
{
	uint8_t n = mem_read8(state, ++state->registers.pc);
	mem_write8(state, state->registers.hl, n);
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief SCF (0x37)
 * @result C flag set
 */
static inline void scf(emu_state *restrict state)
{
	*(state->registers.f) |= FLAG_C;
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief JR C,n (0x38)
 * @result add n to pc if C (carry) flag set
 */
static inline void jr_c_imm8(emu_state *restrict state)
{
	int8_t to_add = mem_read8(state, ++state->registers.pc) + 1;

	state->registers.pc += (*(state->registers.f) & FLAG_C) ? to_add : 1;

	state->wait = 8;
}

/*!
 * @brief ADD HL,SP (0x39)
 * @result HL += SP; N flag reset, H if carry from bit 11, C if overflow
 */
static inline void add_hl_sp(emu_state *restrict state)
{
	add_to_hl(state, state->registers.sp);

	state->wait = 8;
}

/*!
 * @brief LD A,(HL-) (0x3A)
 * @result A = contents of memory at HL; HL decremented 1
 */
static inline void ldd_a_hl(emu_state *restrict state)
{
	*(state->registers.a) = mem_read8(state, state->registers.hl--);
	state->registers.pc++;

	state->wait = 8;
}

/*!
 * @brief DEC SP (0x3B)
 * @result 1 is subtracted from SP (possibly wrapping)
 */
static inline void dec_sp(emu_state *restrict state)
{
	state->registers.sp--;
	state->registers.pc++;

	state->wait = 8;
}

/*!
 * @brief INC A (0x3C)
 * @result 1 is added to A; Z if A is now zero, H if bit 3 overflow
 */
static inline void inc_a(emu_state *restrict state)
{
	inc_r8(state, state->registers.a);
}

/*!
 * @brief DEC A (0x3D)
 * @result 1 is subtracted from A; Z if A is now zero, H if bit 4 underflow
 */
static inline void dec_a(emu_state *restrict state)
{
	dec_r8(state, state->registers.a);
}

/*!
 * @brief LD A,n (0x3E)
 * @result A = n
 */
static inline void ld_a_imm8(emu_state *restrict state)
{
	*(state->registers.a) = mem_read8(state, ++state->registers.pc);
	state->registers.pc++;

	state->wait = 8;
}

/*!
 * @brief CCF (0x3F)
 * @result C flag inverted
 */
static inline void ccf(emu_state *restrict state)
{
	*(state->registers.f) ^= ~FLAG_C;
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief LD B,B (0x40)
 * @result B = B
 */
static inline void ld_b_b(emu_state *restrict state)
{
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief LD B,C (0x41)
 * @result B = C
 */
static inline void ld_b_c(emu_state *restrict state)
{
	*(state->registers.b) = *(state->registers.c);
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief LD B,D (0x42)
 * @result B = D
 */
static inline void ld_b_d(emu_state *restrict state)
{
	*(state->registers.b) = *(state->registers.d);
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief LD B,E (0x43)
 * @result B = E
 */
static inline void ld_b_e(emu_state *restrict state)
{
	*(state->registers.b) = *(state->registers.e);
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief LD B,H (0x44)
 * @result B = H
 */
static inline void ld_b_h(emu_state *restrict state)
{
	*(state->registers.b) = *(state->registers.h);
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief LD B,L (0x45)
 * @result B = L
 */
static inline void ld_b_l(emu_state *restrict state)
{
	*(state->registers.b) = *(state->registers.l);
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief LD B,(HL) (0x46)
 * @result B = contents of memory at HL
 */
static inline void ld_b_hl(emu_state *restrict state)
{
	*(state->registers.b) = mem_read8(state, state->registers.hl);
	state->registers.pc++;

	state->wait = 8;
}

/*!
 * @brief LD B,A (0x47)
 * @result B = A
 */
static inline void ld_b_a(emu_state *restrict state)
{
	*(state->registers.b) = *(state->registers.a);
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief LD C,B (0x48)
 * @result C = B
 */
static inline void ld_c_b(emu_state *restrict state)
{
	*(state->registers.c) = *(state->registers.b);
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief LD C,C (0x49)
 * @result C = C
 */
static inline void ld_c_c(emu_state *restrict state)
{
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief LD C,D (0x4A)
 * @result C = D
 */
static inline void ld_c_d(emu_state *restrict state)
{
	*(state->registers.c) = *(state->registers.d);
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief LD C,E (0x4B)
 * @result C = E
 */
static inline void ld_c_e(emu_state *restrict state)
{
	*(state->registers.c) = *(state->registers.e);
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief LD C,H (0x4C)
 * @result C = H
 */
static inline void ld_c_h(emu_state *restrict state)
{
	*(state->registers.c) = *(state->registers.h);
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief LD C,L (0x4D)
 * @result C = L
 */
static inline void ld_c_l(emu_state *restrict state)
{
	*(state->registers.c) = *(state->registers.l);
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief LD C,(HL) (0x4E)
 * @result C = contents of memory at HL
 */
static inline void ld_c_hl(emu_state *restrict state)
{
	*(state->registers.c) = mem_read8(state, state->registers.hl);
	state->registers.pc++;

	state->wait = 8;
}

/*!
 * @brief LD C,A (0x4F)
 * @result C = A
 */
static inline void ld_c_a(emu_state *restrict state)
{
	*(state->registers.c) = *(state->registers.a);
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief LD D,B (0x50)
 * @result D = B
 */
static inline void ld_d_b(emu_state *restrict state)
{
	*(state->registers.d) = *(state->registers.b);
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief LD D,C (0x51)
 * @result D = C
 */
static inline void ld_d_c(emu_state *restrict state)
{
	*(state->registers.d) = *(state->registers.c);
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief LD D,D (0x52)
 * @result D = D
 */
static inline void ld_d_d(emu_state *restrict state)
{
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief LD D,E (0x53)
 * @result D = E
 */
static inline void ld_d_e(emu_state *restrict state)
{
	*(state->registers.d) = *(state->registers.e);
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief LD D,H (0x54)
 * @result D = H
 */
static inline void ld_d_h(emu_state *restrict state)
{
	*(state->registers.d) = *(state->registers.h);
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief LD D,L (0x55)
 * @result D = L
 */
static inline void ld_d_l(emu_state *restrict state)
{
	*(state->registers.d) = *(state->registers.l);
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief LD D,(HL) (0x56)
 * @result D = contents of memory at HL
 */
static inline void ld_d_hl(emu_state *restrict state)
{
	*(state->registers.d) = mem_read8(state, state->registers.hl);
	state->registers.pc++;

	state->wait = 8;
}

/*!
 * @brief LD D,A (0x57)
 * @result D = A
 */
static inline void ld_d_a(emu_state *restrict state)
{
	*(state->registers.d) = *(state->registers.a);
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief LD E,B (0x58)
 * @result E = B
 */
static inline void ld_e_b(emu_state *restrict state)
{
	*(state->registers.e) = *(state->registers.b);
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief LD E,C (0x59)
 * @result E = C
 */
static inline void ld_e_c(emu_state *restrict state)
{
	*(state->registers.e) = *(state->registers.c);
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief LD E,D (0x5A)
 * @result E = D
 */
static inline void ld_e_d(emu_state *restrict state)
{
	*(state->registers.e) = *(state->registers.d);
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief LD E,E (0x5B)
 * @result E = E
 */
static inline void ld_e_e(emu_state *restrict state)
{
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief LD E,H (0x5C)
 * @result E = H
 */
static inline void ld_e_h(emu_state *restrict state)
{
	*(state->registers.e) = *(state->registers.h);
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief LD E,L (0x5D)
 * @result E = L
 */
static inline void ld_e_l(emu_state *restrict state)
{
	*(state->registers.e) = *(state->registers.l);
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief LD E,(HL) (0x5E)
 * @result E = contents of memory at HL
 */
static inline void ld_e_hl(emu_state *restrict state)
{
	*(state->registers.e) = mem_read8(state, state->registers.hl);
	state->registers.pc++;

	state->wait = 8;
}

/*!
 * @brief LD E,A (0x5F)
 * @result E = A
 */
static inline void ld_e_a(emu_state *restrict state)
{
	*(state->registers.e) = *(state->registers.a);
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief LD H,B (0x60)
 * @result H = B
 */
static inline void ld_h_b(emu_state *restrict state)
{
	*(state->registers.h) = *(state->registers.b);
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief LD H,C (0x61)
 * @result H = C
 */
static inline void ld_h_c(emu_state *restrict state)
{
	*(state->registers.h) = *(state->registers.c);
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief LD H,D (0x62)
 * @result H = D
 */
static inline void ld_h_d(emu_state *restrict state)
{
	*(state->registers.h) = *(state->registers.d);
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief LD H,E (0x63)
 * @result H = E
 */
static inline void ld_h_e(emu_state *restrict state)
{
	*(state->registers.h) = *(state->registers.e);
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief LD H,H (0x64)
 * @result H = H
 */
static inline void ld_h_h(emu_state *restrict state)
{
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief LD H,L (0x65)
 * @result H = L
 */
static inline void ld_h_l(emu_state *restrict state)
{
	*(state->registers.h) = *(state->registers.l);
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief LD H,(HL) (0x66)
 * @result H = contents of memory at HL
 */
static inline void ld_h_hl(emu_state *restrict state)
{
	*(state->registers.h) = mem_read8(state, state->registers.hl);
	state->registers.pc++;

	state->wait = 8;
}

/*!
 * @brief LD H,A (0x67)
 * @result H = A
 */
static inline void ld_h_a(emu_state *restrict state)
{
	*(state->registers.h) = *(state->registers.a);
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief LD L,B (0x68)
 * @result L = B
 */
static inline void ld_l_b(emu_state *restrict state)
{
	*(state->registers.l) = *(state->registers.b);
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief LD L,C (0x69)
 * @result L = C
 */
static inline void ld_l_c(emu_state *restrict state)
{
	*(state->registers.l) = *(state->registers.c);
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief LD L,D (0x6A)
 * @result L = D
 */
static inline void ld_l_d(emu_state *restrict state)
{
	*(state->registers.l) = *(state->registers.d);
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief LD L,E (0x6B)
 * @result L = E
 */
static inline void ld_l_e(emu_state *restrict state)
{
	*(state->registers.l) = *(state->registers.e);
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief LD L,H (0x6C)
 * @result L = H
 */
static inline void ld_l_h(emu_state *restrict state)
{
	*(state->registers.l) = *(state->registers.h);
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief LD L,L (0x6D)
 * @result L = L
 */
static inline void ld_l_l(emu_state *restrict state)
{
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief LD L,(HL) (0x6E)
 * @result L = contents of memory at HL
 */
static inline void ld_l_hl(emu_state *restrict state)
{
	*(state->registers.l) = mem_read8(state, state->registers.hl);
	state->registers.pc++;

	state->wait = 8;
}

/*!
 * @brief LD L,A (0x6F)
 * @result L = A
 */
static inline void ld_l_a(emu_state *restrict state)
{
	*(state->registers.l) = *(state->registers.a);
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief LD (HL),B (0x70)
 * @result contents of memory at HL = B
 */
static inline void ld_hl_b(emu_state *restrict state)
{
	mem_write8(state, state->registers.hl, *(state->registers.b));
	state->registers.pc++;

	state->wait = 8;
}

/*!
 * @brief LD (HL),C (0x71)
 * @result contents of memory at HL = C
 */
static inline void ld_hl_c(emu_state *restrict state)
{
	mem_write8(state, state->registers.hl, *(state->registers.c));
	state->registers.pc++;

	state->wait = 8;
}

/*!
 * @brief LD (HL),D (0x72)
 * @result contents of memory at HL = D
 */
static inline void ld_hl_d(emu_state *restrict state)
{
	mem_write8(state, state->registers.hl, *(state->registers.d));
	state->registers.pc++;

	state->wait = 8;
}

/*!
 * @brief LD (HL),E (0x73)
 * @result contents of memory at HL = E
 */
static inline void ld_hl_e(emu_state *restrict state)
{
	mem_write8(state, state->registers.hl, *(state->registers.e));
	state->registers.pc++;

	state->wait = 8;
}

/*!
 * @brief LD (HL),H (0x74)
 * @result contents of memory at HL = H
 */
static inline void ld_hl_h(emu_state *restrict state)
{
	mem_write8(state, state->registers.hl, *(state->registers.h));
	state->registers.pc++;

	state->wait = 8;
}

/*!
 * @brief LD (HL),L (0x75)
 * @result contents of memory at HL = L
 */
static inline void ld_hl_l(emu_state *restrict state)
{
	mem_write8(state, state->registers.hl, *(state->registers.l));
	state->registers.pc++;

	state->wait = 8;
}

/*!
 * @brief LD (HL),A (0x77)
 * @result contents of memory at HL = A
 */
static inline void ld_hl_a(emu_state *restrict state)
{
	mem_write8(state, state->registers.hl, *(state->registers.a));
	state->registers.pc++;

	state->wait = 8;
}

/*!
 * @brief LD A,B (0x78)
 * @result A = B
 */
static inline void ld_a_b(emu_state *restrict state)
{
	*(state->registers.a) = *(state->registers.b);
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief LD A,C (0x79)
 * @result A = C
 */
static inline void ld_a_c(emu_state *restrict state)
{
	*(state->registers.a) = *(state->registers.c);
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief LD A,D (0x7A)
 * @result A = D
 */
static inline void ld_a_d(emu_state *restrict state)
{
	*(state->registers.a) = *(state->registers.d);
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief LD A,E (0x7B)
 * @result A = E
 */
static inline void ld_a_e(emu_state *restrict state)
{
	*(state->registers.a) = *(state->registers.e);
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief LD A,H (0x7C)
 * @result A = H
 */
static inline void ld_a_h(emu_state *restrict state)
{
	*(state->registers.a) = *(state->registers.h);
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief LD A,L (0x7D)
 * @result A = L
 */
static inline void ld_a_l(emu_state *restrict state)
{
	*(state->registers.a) = *(state->registers.l);
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief LD A,(HL) (0x7E)
 * @result A = contents of memory at HL
 */
static inline void ld_a_hl(emu_state *restrict state)
{
	*(state->registers.a) = mem_read8(state, state->registers.hl);
	state->registers.pc++;

	state->wait = 8;
}

/*!
 * @brief LD A,A (0x7F)
 * @result A = A
 */
static inline void ld_a_a(emu_state *restrict state)
{
	state->registers.pc++;

	state->wait = 4;
}

static inline void add_common(emu_state *restrict state, uint8_t to_add)
{
	uint32_t temp = *(state->registers.a) + to_add;

	*(state->registers.f) = 0;

	if(temp)
	{
		if(temp & 0x100)
		{
			*(state->registers.f) |= FLAG_C;
		}

		// Half carry
		if(((*(state->registers.a) & 0xF) + (to_add & 0xF)) & 0x10)
		{
			*(state->registers.f) |= FLAG_H;
		}
	}
	else
	{
		*(state->registers.f) |= FLAG_Z;
	}

	*(state->registers.a) = (uint8_t)temp;
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief ADD B (0x80)
 * @result A += B
 */
static inline void add_b(emu_state *restrict state)
{
	add_common(state, *(state->registers.b));
}

/*!
 * @brief ADD C (0x81)
 * @result A += C
 */
static inline void add_c(emu_state *restrict state)
{
	add_common(state, *(state->registers.c));
}

/*!
 * @brief ADD D (0x82)
 * @result A += D
 */
static inline void add_d(emu_state *restrict state)
{
	add_common(state, *(state->registers.d));
}

/*!
 * @brief ADD E (0x83)
 * @result A += E
 */
static inline void add_e(emu_state *restrict state)
{
	add_common(state, *(state->registers.e));
}

/*!
 * @brief ADD H (0x84)
 * @result A += H
 */
static inline void add_h(emu_state *restrict state)
{
	add_common(state, *(state->registers.h));
}

/*!
 * @brief ADD L (0x85)
 * @result A += L
 */
static inline void add_l(emu_state *restrict state)
{
	add_common(state, *(state->registers.l));
}

/*!
 * @brief ADD (HL) (0x86)
 * @result A += contents of memory at HL
 */
static inline void add_hl(emu_state *restrict state)
{
	add_common(state, mem_read8(state, state->registers.hl));

	// add_common already adds 4
	state->wait = 4;
}

/*!
 * @brief ADD A (0x87)
 * @result A += A
 */
static inline void add_a(emu_state *restrict state)
{
	add_common(state, *(state->registers.a));
}

static inline void adc_common(emu_state *restrict state, uint8_t to_add)
{
	if(*(state->registers.f) & FLAG_C)
	{
		to_add++;
	}

	add_common(state, to_add);
}

/*!
 * @brief ADC B (0x88)
 * @result A += B (+1 if C flag set)
 */
static inline void adc_b(emu_state *restrict state)
{
	adc_common(state, *(state->registers.b));
}

/*!
 * @brief ADC C (0x89)
 * @result A += C (+1 if C flag set)
 */
static inline void adc_c(emu_state *restrict state)
{
	adc_common(state, *(state->registers.c));
}

/*!
 * @brief ADC D (0x8A)
 * @result A += D (+1 if C flag set)
 */
static inline void adc_d(emu_state *restrict state)
{
	adc_common(state, *(state->registers.d));
}

/*!
 * @brief ADC E (0x8B)
 * @result A += E (+1 if C flag set)
 */
static inline void adc_e(emu_state *restrict state)
{
	adc_common(state, *(state->registers.e));
}

/*!
 * @brief ADC H (0x8C)
 * @result A += H (+1 if C flag set)
 */
static inline void adc_h(emu_state *restrict state)
{
	adc_common(state, *(state->registers.h));
}

/*!
 * @brief ADC L (0x8D)
 * @result A += L (+1 if C flag set)
 */
static inline void adc_l(emu_state *restrict state)
{
	adc_common(state, *(state->registers.l));
}

/*!
 * @brief ADC (HL) (0x8E)
 * @result A += contents of memory at HL (+1 if C flag set)
 */
static inline void adc_hl(emu_state *restrict state)
{
	adc_common(state, mem_read8(state, state->registers.hl));

	// adc_common already adds 4
	state->wait += 4;
}

/*!
 * @brief ADC A (0x8F)
 * @result A += A (+1 if C flag set)
 */
static inline void adc_a(emu_state *restrict state)
{
	adc_common(state, *(state->registers.a));
}

static inline void sub_common(emu_state *restrict state, uint8_t to_sub)
{
	add_common(state, ~to_sub + 1);
}

/*!
 * @brief SUB B (0x90)
 * @result A -= B; Z if A = 0, H if no borrow from bit 4, C if no borrow
 */
static inline void sub_b(emu_state *restrict state)
{
	sub_common(state, *(state->registers.b));
}

/*!
 * @brief SUB C (0x91)
 * @result A -= C; Z if A = 0, H if no borrow from bit 4, C if no borrow
 */
static inline void sub_c(emu_state *restrict state)
{
	sub_common(state, *(state->registers.c));
}

/*!
 * @brief SUB D (0x92)
 * @result A -= D; Z if A = 0, H if no borrow from bit 4, C if no borrow
 */
static inline void sub_d(emu_state *restrict state)
{
	sub_common(state, *(state->registers.d));
}

/*!
 * @brief SUB E (0x93)
 * @result A -= E; Z if A = 0, H if no borrow from bit 4, C if no borrow
 */
static inline void sub_e(emu_state *restrict state)
{
	sub_common(state, *(state->registers.e));
}

/*!
 * @brief SUB H (0x94)
 * @result A -= H; Z if A = 0, H if no borrow from bit 4, C if no borrow
 */
static inline void sub_h(emu_state *restrict state)
{
	sub_common(state, *(state->registers.h));
}

/*!
 * @brief SUB L (0x95)
 * @result A -= L; Z if A = 0, H if no borrow from bit 4, C if no borrow
 */
static inline void sub_l(emu_state *restrict state)
{
	sub_common(state, *(state->registers.l));
}

/*!
 * @brief SUB (HL) (0x96)
 * @result A -= contents of memory at HL
 */
static inline void sub_hl(emu_state *restrict state)
{
	sub_common(state, mem_read8(state, state->registers.hl));

	// sub_common already adds 4
	state->wait += 4;
}

/*!
 * @brief SUB A (0x97)
 * @result A = 0; Z set, H if no borrow from bit 4, C if no borrow
 */
static inline void sub_a(emu_state *restrict state)
{
	sub_common(state, *(state->registers.a));
}


static inline void sbc_common(emu_state *restrict state, uint8_t to_sub)
{
	if(*(state->registers.f) & FLAG_C)
	{
		to_sub++;
	}

	sub_common(state, to_sub);
}

/*!
 * @brief SBC B (0x98)
 * @result A -= B (+1 if C flag set)
 */
static inline void sbc_b(emu_state *restrict state)
{
	sbc_common(state, *(state->registers.b));
}

/*!
 * @brief SBC C (0x99)
 * @result A -= C (+1 if C flag set)
 */
static inline void sbc_c(emu_state *restrict state)
{
	sbc_common(state, *(state->registers.c));
}

/*!
 * @brief SBC D (0x9A)
 * @result A -= D (+1 if C flag set)
 */
static inline void sbc_d(emu_state *restrict state)
{
	sbc_common(state, *(state->registers.d));
}

/*!
 * @brief SBC E (0x9B)
 * @result A -= E (+1 if C flag set)
 */
static inline void sbc_e(emu_state *restrict state)
{
	sbc_common(state, *(state->registers.e));
}

/*!
 * @brief SBC H (0x9C)
 * @result A -= H (+1 if C flag set)
 */
static inline void sbc_h(emu_state *restrict state)
{
	sbc_common(state, *(state->registers.h));
}

/*!
 * @brief SBC L (0x9D)
 * @result A -= L (+1 if C flag set)
 */
static inline void sbc_l(emu_state *restrict state)
{
	sbc_common(state, *(state->registers.l));
}

/*!
 * @brief SBC (HL) (0x9E)
 * @result A -= contents of memory at HL (+1 if C flag set)
 */
static inline void sbc_hl(emu_state *restrict state)
{
	sbc_common(state, mem_read8(state, state->registers.hl));

	// sbc_common already adds 4
	state->wait += 4;
}

/*!
 * @brief SBC A (0x9F)
 * @result A -= A (+1 if C flag set)
 */
static inline void sbc_a(emu_state *restrict state)
{
	sbc_common(state, *(state->registers.a));
}

static inline void and_common(emu_state *restrict state, uint8_t to_and)
{
	*(state->registers.a) &= to_and;

	*(state->registers.f) = FLAG_H;

	if(!*(state->registers.a))
	{
		*(state->registers.f) |= FLAG_Z;
	}

	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief AND B (0xA0)
 * @result A &= B; Z flag set if A is now zero
 */
static inline void and_b(emu_state *restrict state)
{
	and_common(state, *(state->registers.b));
}

/*!
 * @brief AND C (0xA1)
 * @result A &= C; Z flag set if A is now zero
 */
static inline void and_c(emu_state *restrict state)
{
	and_common(state, *(state->registers.c));
}

/*!
 * @brief AND D (0xA2)
 * @result A &= D; Z flag set if A is now zero
 */
static inline void and_d(emu_state *restrict state)
{
	and_common(state, *(state->registers.d));
}

/*!
 * @brief AND E (0xA3)
 * @result A &= E; Z flag set if A is now zero
 */
static inline void and_e(emu_state *restrict state)
{
	and_common(state, *(state->registers.e));
}

/*!
 * @brief AND H (0xA4)
 * @result A &= H; Z flag set if A is now zero
 */
static inline void and_h(emu_state *restrict state)
{
	and_common(state, *(state->registers.h));
}

/*!
 * @brief AND L (0xA5)
 * @result A &= L; Z flag set if A is now zero
 */
static inline void and_l(emu_state *restrict state)
{
	and_common(state, *(state->registers.l));
}

/*!
 * @brief AND (HL) (0xA6)
 * @result A &= contents of memory at AL; Z flag set if A is now zero
 */
static inline void and_hl(emu_state *restrict state)
{
	and_common(state, mem_read8(state, state->registers.hl));

	// and_common already adds 4
	state->wait += 4;
}

/*!
 * @brief AND A (0xA7)
 * @result Z flag set if A is now zero
 */
static inline void and_a(emu_state *restrict state)
{
	*(state->registers.f) = FLAG_H;
	if(*(state->registers.a) == 0)
	{
		*(state->registers.f) |= FLAG_Z;
	}

	state->registers.pc++;

	state->wait = 4;
}

static inline void xor_common(emu_state *restrict state, char to_xor)
{
	*(state->registers.a) ^= to_xor;

	*(state->registers.f) = 0;
	if(*(state->registers.a) == 0) *(state->registers.f) |= FLAG_Z;

	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief XOR B (0xA8)
 * @result A ^= B; Z flag set if A is now zero
 */
static inline void xor_b(emu_state *restrict state)
{
	xor_common(state, *(state->registers.b));
}

/*!
 * @brief XOR C (0xA9)
 * @result A ^= C; Z flag set if A is now zero
 */
static inline void xor_c(emu_state *restrict state)
{
	xor_common(state, *(state->registers.c));
}

/*!
 * @brief XOR D (0xAA)
 * @result A ^= D; Z flag set if A is now zero
 */
static inline void xor_d(emu_state *restrict state)
{
	xor_common(state, *(state->registers.d));
}

/*!
 * @brief XOR E (0xAB)
 * @result A ^= E; Z flag set if A is now zero
 */
static inline void xor_e(emu_state *restrict state)
{
	xor_common(state, *(state->registers.e));
}

/*!
 * @brief XOR H (0xAC)
 * @result A ^= H; Z flag set if A is now zero
 */
static inline void xor_h(emu_state *restrict state)
{
	xor_common(state, *(state->registers.h));
}

/*!
 * @brief XOR L (0xAD)
 * @result A ^= L; Z flag set if A is now zero
 */
static inline void xor_l(emu_state *restrict state)
{
	xor_common(state, *(state->registers.l));
}

/*!
 * @brief XOR (HL) (0xAE)
 * @result A ^= contents of memory at HL; Z flag set if A is now zero
 */
static inline void xor_hl(emu_state *restrict state)
{
	xor_common(state, mem_read8(state, state->registers.hl));

	// xor_common already adds 4
	state->wait += 4;
}

/*!
 * @brief XOR A (0xAF)
 * @result A = 0; Z flag set
 */
static inline void xor_a(emu_state *restrict state)
{
	*(state->registers.a) = 0;
	*(state->registers.f) = FLAG_Z;
	state->registers.pc++;

	state->wait = 4;
}

static inline void or_common(emu_state *restrict state, uint8_t to_or)
{
	*(state->registers.a) |= to_or;

	*(state->registers.f) = (*(state->registers.a) == 0 ? FLAG_Z : 0);

	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief OR B (0xB0)
 * @result A |= B; Z flag set if A is zero
 */
static inline void or_b(emu_state *restrict state)
{
	or_common(state, *(state->registers.b));
}

/*!
 * @brief OR C (0xB1)
 * @result A |= C; Z flag set if A is zero
 */
static inline void or_c(emu_state *restrict state)
{
	or_common(state, *(state->registers.c));
}

/*!
 * @brief OR D (0xB2)
 * @result A |= D; Z flag set if A is zero
 */
static inline void or_d(emu_state *restrict state)
{
	or_common(state, *(state->registers.d));
}

/*!
 * @brief OR E (0xB3)
 * @result A |= E; Z flag set if A is zero
 */
static inline void or_e(emu_state *restrict state)
{
	or_common(state, *(state->registers.e));
}

/*!
 * @brief OR H (0xB4)
 * @result A |= H; Z flag set if A is zero
 */
static inline void or_h(emu_state *restrict state)
{
	or_common(state, *(state->registers.h));
}

/*!
 * @brief OR L (0xB5)
 * @result A |= L; Z flag set if A is zero
 */
static inline void or_l(emu_state *restrict state)
{
	or_common(state, *(state->registers.l));
}

/*!
 * @brief OR (HL) (0xB6)
 * @result A |= contents of memory at HL; Z flag set if A is zero
 */
static inline void or_hl(emu_state *restrict state)
{
	or_common(state, mem_read8(state, state->registers.hl));

	// or_common already adds 4
	state->wait += 4;
}

/*!
 * @brief OR A (0xB7)
 * @result A |= A; Z flag set if A is zero
 */
static inline void or_a(emu_state *restrict state)
{
	or_common(state, *(state->registers.a));
}

static inline void cp_common(emu_state *restrict state, uint8_t cmp)
{
	*(state->registers.f) = FLAG_N;
	if(*(state->registers.a) == cmp)
	{
		*(state->registers.f) |= FLAG_Z;
	}
	else
	{
		if(*(state->registers.a) < cmp)
		{
			*(state->registers.f) |= FLAG_C;
		}
		else
		{
			*(state->registers.f) |= FLAG_H;
		}
	}

	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief CP B (0xB8)
 * @result flags set based on how equivalent B is to A
 */
static inline void cp_b(emu_state *restrict state)
{
	cp_common(state, *(state->registers.b));
}

/*!
 * @brief CP C (0xB9)
 * @result flags set based on how equivalent C is to A
 */
static inline void cp_c(emu_state *restrict state)
{
	cp_common(state, *(state->registers.c));
}

/*!
 * @brief CP D (0xBA)
 * @result flags set based on how equivalent D is to A
 */
static inline void cp_d(emu_state *restrict state)
{
	cp_common(state, *(state->registers.d));
}

/*!
 * @brief CP E (0xBB)
 * @result flags set based on how equivalent E is to A
 */
static inline void cp_e(emu_state *restrict state)
{
	cp_common(state, *(state->registers.e));
}

/*!
 * @brief CP H (0xBC)
 * @result flags set based on how equivalent H is to A
 */
static inline void cp_h(emu_state *restrict state)
{
	cp_common(state, *(state->registers.h));
}

/*!
 * @brief CP L (0xBD)
 * @result flags set based on how equivalent L is to A
 */
static inline void cp_l(emu_state *restrict state)
{
	cp_common(state, *(state->registers.l));
}

/*!
 * @brief CP (HL) (0xBE)
 * @result flags set based on how equivalent contents of memory at HL are to A
 */
static inline void cp_hl(emu_state *restrict state)
{
	cp_common(state, mem_read8(state, state->registers.hl));

	// cp_common already adds 4
	state->wait += 4;
}

/*!
 * @brief CP A (0xBF)
 * @result flags set based on how equivalent A is to A... wait.. really?
 */
static inline void cp_a(emu_state *restrict state)
{
	cp_common(state, *(state->registers.a));
}

/*!
 * @brief POP BC (0xC1)
 * @result BC = memory at SP; SP incremented 2
 */
static inline void pop_bc(emu_state *restrict state)
{
	state->registers.bc = mem_read16(state, state->registers.sp);
	state->registers.sp += 2;
	state->registers.pc++;

	state->wait = 12;
}

/*!
 * @brief JP NZ,nn (0xC2)
 * @result pc is set to 16-bit immediate value (LSB, MSB) if Z flag is not set
 */
static inline void jp_nz_imm16(emu_state *restrict state)
{
	uint8_t lsb = mem_read8(state, ++state->registers.pc);
	uint8_t msb = mem_read8(state, ++state->registers.pc);

	state->registers.pc = (*(state->registers.f) & FLAG_Z) ?
		state->registers.pc+1 : (msb<<8 | lsb);

	state->wait = 12;
}

/*!
 * @brief JP nn (0xC3)
 * @result pc is set to 16-bit immediate value (LSB, MSB)
 */
static inline void jp_imm16(emu_state *restrict state)
{
	uint8_t lsb = mem_read8(state, ++state->registers.pc);
	uint8_t msb = mem_read8(state, ++state->registers.pc);

	state->registers.pc = (msb<<8 | lsb);

	state->wait = 16;
}

/*!
 * @brief PUSH BC (0xC5)
 * @result contents of memory at SP = BC; SP decremented 2
 */
static inline void push_bc(emu_state *restrict state)
{
	state->registers.sp -= 2;
	mem_write16(state, state->registers.sp, state->registers.bc);
	state->registers.pc++;

	state->wait = 16;
}

/*!
 * @brief ADD n (0xC6)
 * @result A += immediate (n)
 */
static inline void add_imm8(emu_state *restrict state)
{
	add_common(state, mem_read8(state, ++state->registers.pc));

	// add_common already adds 4
	state->wait += 4;
}

static inline void reset_common(emu_state *restrict state)
{
	uint16_t to = mem_read8(state, state->registers.pc) - 0xC7;

	state->registers.sp -= 2;
	mem_write16(state, state->registers.sp, ++state->registers.pc);
	state->registers.pc = to;

	state->wait = 16;
}

/*!
 * @brief RET (0xC9) - return from CALL
 * @result pop two bytes from the stack and jump to that location
 */
static inline void ret(emu_state *restrict state)
{
	state->registers.pc = mem_read16(state, state->registers.sp);
	state->registers.sp += 2;

	state->wait = 16;
}

/*!
 * @brief RETNZ (0xC0) - return from CALL if Z flag not set
 * @result RET, if Z flag not set, otherwise nothing.
 */
static inline void retnz(emu_state *restrict state)
{
	if(*(state->registers.f) & FLAG_Z)
	{
		state->registers.pc++;

		state->wait = 8;
	}
	else
	{
		ret(state);

		// ret already adds 16
		state->wait += 4;
	}
}

/*!
 * @brief RETZ (0xC8) - return from CALL if Z flag set
 * @result RET, if Z flag is set, otherwise nothing.
 */
static inline void retz(emu_state *restrict state)
{
	if(*(state->registers.f) & FLAG_Z)
	{
		ret(state);

		// ret already adds 16
		state->wait += 4;
	}
	else
	{
		state->registers.pc++;

		state->wait = 8;
	}
}

/*!
 * @brief JP Z,nn (0xCA)
 * @result pc is set to 16-bit immediate value (LSB, MSB) if Z flag is set
 */
static inline void jp_z_imm16(emu_state *restrict state)
{
	uint8_t lsb = mem_read8(state, ++state->registers.pc);
	uint8_t msb = mem_read8(state, ++state->registers.pc);

	state->registers.pc = (*(state->registers.f) & FLAG_Z) ? (msb<<8 | lsb) : state->registers.pc+1;

	state->wait = 12;
}

/*!
 * @brief CB ..
 * @note this is just a dispatch function for SWAP/BIT/etc
 */
static inline void cb_dispatch(emu_state *restrict state)
{
	int8_t opcode = mem_read8(state, ++state->registers.pc);
	uint8_t *write_to;
	uint8_t maybe_temp;
	uint8_t bit_number;
	cb_regs reg = (cb_regs)(opcode & 0x7);
	cb_ops op;

	state->wait = 4;

	// FIXME additional cycles depending on CB instruction

	if(opcode >= 0x40)
	{
		bit_number = (opcode & 0x38) >> 3;
		op = (cb_ops)(((opcode & 0xC0) >> 7) + 8);
	}
	else
	{
		bit_number = 0;
		op = (cb_ops)(opcode >> 3);
	}

	switch(reg)
	{
	case CB_REG_B:
		write_to = state->registers.b;
		break;
	case CB_REG_C:
		write_to = state->registers.c;
		break;
	case CB_REG_D:
		write_to = state->registers.d;
		break;
	case CB_REG_E:
		write_to = state->registers.e;
		break;
	case CB_REG_H:
		write_to = state->registers.h;
		break;
	case CB_REG_L:
		write_to = state->registers.l;
		break;
	case CB_REG_HL:
		write_to = &maybe_temp;
		break;
	case CB_REG_A:
		write_to = state->registers.a;
		break;
	}

	uint8_t val = (1 << bit_number);

	if(reg == CB_REG_HL)
	{
		maybe_temp = mem_read8(state, state->registers.hl);
	}

	switch(op)
	{
	case CB_OP_RLC:
		if(*write_to & 0x80)
		{
			*(state->registers.f) = FLAG_C;
		}
		else
		{
			*(state->registers.f) = 0x00;
		}

		*write_to <<= 1;
		*write_to |= ((*(state->registers.f) & FLAG_C) == FLAG_C);

		if(*write_to == 0)
		{
			*(state->registers.f) |= FLAG_Z;
		}

		break;
	case CB_OP_RRC:
		if(*write_to & 0x01)
		{
			*(state->registers.f) = FLAG_C;
		}
		else
		{
			*(state->registers.f) = 0x00;
		}

		*write_to >>= 1;
		if(*(state->registers.f) & FLAG_C)
		{
			*write_to |= 0x80;
		}

		if(*write_to == 0)
		{
			*(state->registers.f) |= FLAG_Z;
		}

		break;
	case CB_OP_RL:
		/* abusing FLAG_H as a temp var. */
		if(*write_to & 0x80)
		{
			*(state->registers.f) = FLAG_H;
		}
		else
		{
			*(state->registers.f) = 0x00;
		}

		*write_to <<= 1;
		*write_to |= ((*(state->registers.f) & FLAG_C) == FLAG_C);

		if(*(state->registers.f) & FLAG_H)
		{
			*(state->registers.f) = FLAG_C;
		}

		if(*write_to == 0)
		{
			*(state->registers.f) |= FLAG_Z;
		}

		break;
	case CB_OP_RR:
		/* same as above */
		if(*write_to & 0x01)
		{
			*(state->registers.f) = FLAG_H;
		}
		else
		{
			*(state->registers.f) = 0x00;
		}

		*write_to >>= 1;
		*write_to |= ((*(state->registers.f) & FLAG_C) == FLAG_C);

		if(*(state->registers.f) & FLAG_H)
		{
			*(state->registers.f) = FLAG_C;
		}

		if(*write_to == 0)
		{
			*(state->registers.f) |= FLAG_Z;
		}

		break;
	case CB_OP_SLA:
		if(*write_to & 0x80)
		{
			*(state->registers.f) = FLAG_C;
		}
		else
		{
			*(state->registers.f) = 0x00;
		}

		*write_to <<= 1;

		if(*write_to == 0)
		{
			*(state->registers.f) |= FLAG_Z;
		}

		break;
	case CB_OP_SRA:
		if(*write_to & 0x01)
		{
			*(state->registers.f) = FLAG_C;
		}
		else
		{
			*(state->registers.f) = 0x00;
		}

		*write_to = (*write_to & 0x80) | (*write_to >> 1);

		if(*write_to == 0)
		{
			*(state->registers.f) |= FLAG_Z;
		}

		break;
	case CB_OP_SRL:
		if(*write_to & 0x01)
		{
			*(state->registers.f) = FLAG_C;
		}
		else
		{
			*(state->registers.f) = 0x00;
		}

		*write_to >>= 1;

		if(*write_to == 0)
		{
			*(state->registers.f) |= FLAG_Z;
		}

		break;
	case CB_OP_RES:
		/* reset bit <bit_number> of register <reg> */
		*write_to &= ~val;
		break;
	case CB_OP_SET:
		/* set bit <bit_number> of register <reg> */
		*write_to |= val;
		break;
	case CB_OP_BIT:
		/* test bit <bit_number> of register <reg> */
		*(state->registers.f) |= (*write_to & val) ? FLAG_Z : !FLAG_Z;
		*(state->registers.f) |= FLAG_H;
		*(state->registers.f) &= !FLAG_N;
		break;
	}

	if(reg == CB_REG_HL)
	{
		mem_write8(state, state->registers.hl, maybe_temp);
	}

	state->registers.pc++;
}

/*!
 * @brief CALL nn (0xCD)
 * @result next pc stored in stack; jump to nn
 */
static inline void call_imm16(emu_state *restrict state)
{
	uint8_t lsb = mem_read8(state, ++state->registers.pc);
	uint8_t msb = mem_read8(state, ++state->registers.pc);

	state->registers.sp -= 2;
	mem_write16(state, state->registers.sp, ++state->registers.pc);

	state->registers.pc = (msb<<8 | lsb);

	state->wait = 24;
}

/*!
 * @brief RETNC (0xD0) - return from CALL if C flag not set
 * @result RET, if C flag not set, otherwise nothing.
 */
static inline void retnc(emu_state *restrict state)
{
	if(*(state->registers.f) & FLAG_C)
	{
		state->registers.pc++;

		state->wait = 8;
	}
	else
	{
		ret(state);

		// ret already adds 16
		state->wait += 4;
	}
}

/*!
 * @brief POP DE (0xD1)
 * @result DE = memory at SP; SP incremented 2
 */
static inline void pop_de(emu_state *restrict state)
{
	state->registers.de = mem_read16(state, state->registers.sp);
	state->registers.sp += 2;
	state->registers.pc++;

	state->wait = 12;
}

/*!
 * @brief JP NC,nn (0xD2)
 * @result pc is set to 16-bit immediate value (LSB, MSB) if C flag is not set
 */
static inline void jp_nc_imm16(emu_state *restrict state)
{
	uint8_t lsb = mem_read8(state, ++state->registers.pc);
	uint8_t msb = mem_read8(state, ++state->registers.pc);

	state->registers.pc = (*(state->registers.f) & FLAG_C) ? state->registers.pc+1 : (msb<<8 | lsb);

	state->wait = 12;
}

/*!
 * @brief PUSH DE (0xD5)
 * @result contents of memory at SP = DE; SP decremented 2
 */
static inline void push_de(emu_state *restrict state)
{
	state->registers.sp -= 2;
	mem_write16(state, state->registers.sp, state->registers.de);
	state->registers.pc++;

	state->wait = 16;
}

/*!
 * @brief SUB n (0xD6)
 * @result A -= n; Z if A = 0, H if no borrow from bit 4, C if no borrow
 */
static inline void sub_imm8(emu_state *restrict state)
{
	sub_common(state, mem_read8(state, ++state->registers.pc));

	// sub_common already adds 4
	state->wait += 4;
}

/*!
 * @brief RETC (0xD8) - return from CALL if C flag set
 * @result RET, if C flag is set, otherwise nothing.
 */
static inline void retc(emu_state *restrict state)
{
	if(*(state->registers.f) & FLAG_C)
	{
		ret(state);

		// ret already adds 16
		state->wait += 4;
	}
	else
	{
		state->registers.pc++;

		state->wait = 8;
	}
}

/*!
 * @brief RETI (0xD9) - return from CALL and enable interrupts
 * @result RET + EI
 */
static inline void reti(emu_state *restrict state)
{
	state->iflags |= I_ENABLE_INT_ON_NEXT;
	ret(state);
}

/*!
 * @brief JP C,nn (0xDA)
 * @result pc is set to 16-bit immediate value (LSB, MSB) if C flag is set
 */
static inline void jp_c_imm16(emu_state *restrict state)
{
	uint8_t lsb = mem_read8(state, ++state->registers.pc);
	uint8_t msb = mem_read8(state, ++state->registers.pc);

	state->registers.pc = (*(state->registers.f) & FLAG_C) ? (msb<<8 | lsb)
		: state->registers.pc+1;

	state->wait = 12;
}

/*!
 * @brief LDH n,A (0xE0) - write A to 0xff00+n
 * @result the I/O register n will contain the value of A
 */
static inline void ldh_imm8_a(emu_state *restrict state)
{
	uint16_t write = mem_read8(state, ++state->registers.pc);
	write += 0xFF00;

	mem_write8(state, write, *(state->registers.a));

	state->registers.pc++;

	state->wait = 12;
}

/*!
 * @brief POP HL (0xE1)
 * @result HL = memory at SP; SP incremented 2
 */
static inline void pop_hl(emu_state *restrict state)
{
	state->registers.hl = mem_read16(state, state->registers.sp);
	state->registers.sp += 2;
	state->registers.pc++;

	state->wait = 12;
}

/*!
 * @brief LD (C),A (0xE2)
 * @result contents of memory at 0xFF00 + C = A
 */
static inline void ld_ff00_c_a(emu_state *restrict state)
{
	mem_write8(state, 0xFF00 + *(state->registers.c), *(state->registers.a));
	state->registers.pc++;

	state->wait = 8;
}

/*!
 * @brief PUSH HL (0xE5)
 * @result contents of memory at SP = HL; SP decremented 2
 */
static inline void push_hl(emu_state *restrict state)
{
	state->registers.sp -= 2;
	mem_write16(state, state->registers.sp, state->registers.hl);
	state->registers.pc++;

	state->wait = 16;
}

/*!
 * @brief AND nn (0xE6)
 * @result A &= nn
 */
static inline void and_imm8(emu_state *restrict state)
{
	uint8_t nn = mem_read8(state, ++state->registers.pc);
	and_common(state, nn);

	// and_common already adds 4
	state->wait += 4;
}

/*!
 * @brief JP HL (0xE9)
 * @result pc = HL
 */
static inline void jp_hl(emu_state *restrict state)
{
	state->registers.pc = state->registers.hl;

	state->wait = 4;
}

/*!
 * @brief LD (nn),A (0xEA) - write A to *nn
 * @result the memory at address nn will contain the value of A
 */
static inline void ld_d16_a(emu_state *restrict state)
{
	uint8_t lsb = mem_read8(state, ++state->registers.pc);
	uint8_t msb = mem_read8(state, ++state->registers.pc);

	uint16_t loc = (msb<<8) | lsb;

	mem_write8(state, loc, *(state->registers.a));

	state->registers.pc++;

	state->wait = 16;
}

/*!
 * @brief LDH A,nn (0xF0) - read 0xff00+n to A
 * @result A will contain the value of the I/O register n
 */
static inline void ldh_a_imm8(emu_state *restrict state)
{
	uint8_t loc = mem_read8(state, ++state->registers.pc);
	*(state->registers.a) = mem_read8(state, 0xFF00 + loc);

	state->registers.pc++;

	state->wait = 12;
}

/*!
 * @brief POP AF (0xF1)
 * @result AF = memory at SP; SP incremented 2
 */
static inline void pop_af(emu_state *restrict state)
{
	state->registers.af = mem_read16(state, state->registers.sp);
	state->registers.sp += 2;
	state->registers.pc++;

	state->wait = 12;
}

/*!
 * @brief LD A,(C) (0xF2)
 * @result A = contents of memory at 0xFF00 + C
 */
static inline void ld_a_ff00_c(emu_state *restrict state)
{
	*(state->registers.a) = mem_read8(state, 0xFF00 + *(state->registers.c));
	state->registers.pc++;

	state->wait = 8;
}

/*!
 * @brief DI (0xF3) - disable interrupts
 * @result interrupts will be disabled the instruction AFTER this one
 */
static inline void di(emu_state *restrict state)
{
	state->iflags |= I_DISABLE_INT_ON_NEXT;
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief PUSH AF (0xF5)
 * @result contents of memory at SP = AF; SP decremented 2
 */
static inline void push_af(emu_state *restrict state)
{
	state->registers.sp -= 2;
	mem_write16(state, state->registers.sp, state->registers.af);
	state->registers.pc++;

	state->wait = 16;
}

/*!
 * @brief LD A,(nn) (0xFA) - write *nn to A
 * @result A will contain the value of memory at address nn
 */
static inline void ld_a_d16(emu_state *restrict state)
{
	uint8_t lsb = mem_read8(state, ++state->registers.pc);
	uint8_t msb = mem_read8(state, ++state->registers.pc);

	uint16_t loc = (msb<<8) | lsb;

	*(state->registers.a) = mem_read8(state, loc);

	state->registers.pc++;

	state->wait = 16;
}

/*!
 * @brief EI (0xFB) - enable interrupts
 * @result interrupts will be enabled the instruction AFTER this one
 */
static inline void ei(emu_state *restrict state)
{
	state->iflags |= I_ENABLE_INT_ON_NEXT;
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief CP n (0xFE) - compare A with 8-bit immediate value
 * @result flags register modified based on result
 */
static inline void cp_imm8(emu_state *restrict state)
{
	cp_common(state, mem_read8(state, ++state->registers.pc));

	// cp_common already adds 4
	state->wait += 4;
}

// XXX this is exported for main
static const opcode_t const handlers[0x100] =
{
	/* 0x00 */ nop, ld_bc_imm16, ld_bc_a, inc_bc, inc_b, dec_b, ld_b_imm8, not_impl,
	/* 0x08 */ not_impl, add_hl_bc, ld_a_bc, dec_bc, inc_c, dec_c, ld_c_imm8, not_impl,
	/* 0x10 */ not_impl, ld_de_imm16, ld_de_a, inc_de, inc_d, dec_d, ld_d_imm8, not_impl,
	/* 0x18 */ jr_imm8, add_hl_de, ld_a_de, dec_de, inc_e, dec_e, ld_e_imm8, not_impl,
	/* 0x20 */ jr_nz_imm8, ld_hl_imm16, ldi_hl_a, inc_hl, inc_h, dec_h, ld_h_imm8, not_impl,
	/* 0x28 */ jr_z_imm8, add_hl_hl, ldi_a_hl, dec_hl, inc_l, dec_l, ld_l_imm8, cpl,
	/* 0x30 */ jr_nc_imm8, ld_sp_imm16, ldd_hl_a, inc_sp, inc_hl, dec_hl, ld_hl_imm8, scf,
	/* 0x38 */ jr_c_imm8, add_hl_sp, ldd_a_hl, dec_sp, inc_a, dec_a, ld_a_imm8, ccf,
	/* 0x40 */ ld_b_b, ld_b_c, ld_b_d, ld_b_e, ld_b_h, ld_b_l, ld_b_hl, ld_b_a,
	/* 0x48 */ ld_c_b, ld_c_c, ld_c_d, ld_c_e, ld_c_h, ld_c_l, ld_c_hl, ld_c_a,
	/* 0x50 */ ld_d_b, ld_d_c, ld_d_d, ld_d_e, ld_d_h, ld_d_l, ld_d_hl, ld_d_a,
	/* 0x58 */ ld_e_b, ld_e_c, ld_e_d, ld_e_e, ld_e_h, ld_e_l, ld_e_hl, ld_e_a,
	/* 0x60 */ ld_h_b, ld_h_c, ld_h_d, ld_h_e, ld_h_h, ld_h_l, ld_h_hl, ld_h_a,
	/* 0x68 */ ld_l_b, ld_l_c, ld_l_d, ld_l_e, ld_l_h, ld_l_l, ld_l_hl, ld_l_a,
	/* 0x70 */ ld_hl_b, ld_hl_c, ld_hl_d, ld_hl_e, ld_hl_h, ld_hl_l, not_impl, ld_hl_a,
	/* 0x78 */ ld_a_b, ld_a_c, ld_a_d, ld_a_e, ld_a_h, ld_a_l, ld_a_hl, ld_a_a,
	/* 0x80 */ add_b, add_c, add_d, add_e, add_h, add_l, add_hl, add_a,
	/* 0x88 */ adc_b, adc_c, adc_d, adc_e, adc_h, adc_h, adc_hl, adc_a,
	/* 0x90 */ sub_b, sub_c, sub_d, sub_e, sub_h, sub_l, sub_hl, sub_a,
	/* 0x98 */ sbc_b, sbc_c, sbc_d, sbc_e, sbc_h, sbc_l, sbc_hl, sbc_a,
	/* 0xA0 */ and_b, and_c, and_d, and_e, and_h, and_l, and_hl, and_a,
	/* 0xA8 */ xor_b, xor_c, xor_d, xor_e, xor_h, xor_l, xor_hl, xor_a,
	/* 0xB0 */ or_b, or_c, or_d, or_e, or_h, or_l, or_hl, or_a,
	/* 0xB8 */ cp_b, cp_c, cp_d, cp_e, cp_h, cp_l, cp_hl, cp_a,
	/* 0xC0 */ retnz, pop_bc, jp_nz_imm16, jp_imm16, not_impl, push_bc, add_imm8, reset_common,
	/* 0xC8 */ retz, ret, jp_z_imm16, cb_dispatch, not_impl, call_imm16, not_impl, reset_common,
	/* 0xD0 */ retnc, pop_de, jp_nc_imm16, invalid, not_impl, push_de, sub_imm8, reset_common,
	/* 0xD8 */ retc, reti, jp_c_imm16, invalid, not_impl, invalid, not_impl, reset_common,
	/* 0xE0 */ ldh_imm8_a, pop_hl, ld_ff00_c_a, invalid, invalid, push_hl, and_imm8, reset_common,
	/* 0xE8 */ not_impl, jp_hl, ld_d16_a, invalid, invalid, invalid, not_impl, reset_common,
	/* 0xF0 */ ldh_a_imm8, pop_af, ld_a_ff00_c, di, invalid, push_af, not_impl, reset_common,
	/* 0xF8 */ not_impl, not_impl, ld_a_d16, ei, invalid, invalid, cp_imm8, reset_common
};


/*! boot up */
void init_ctl(emu_state *restrict state, system_types type)
{
	state->registers.pc = 0x0100;
	switch(type)
	{
	case SYSTEM_SGB:
		debug("Super Game Boy emulation");
		*(state->registers.a) = 0x01;
		break;
	case SYSTEM_GBC:
		debug("Game Boy Color emulation");
		*(state->registers.a) = 0x11;
		break;
	case SYSTEM_GBP:
		debug("Game Boy Portable emulation");
		*(state->registers.a) = 0xFF;
		break;
	case SYSTEM_GB:
	default:
		debug("original Game Boy emulation");
		*(state->registers.a) = 0x01;
		break;
	}
	*(state->registers.f) = 0xB0;
	*(state->registers.b) = 0x00;
	*(state->registers.c) = 0x13;
	*(state->registers.d) = 0x00;
	*(state->registers.e) = 0xD8;
	*(state->registers.h) = 0x01;
	*(state->registers.l) = 0x4D;
	state->registers.sp = 0xFFFE;
}


/*! the emulated CU for the 'z80-ish' CPU */
bool execute(emu_state *restrict state)
{
	uint8_t opcode;
	opcode_t handler;
	bool enable = state->iflags & I_ENABLE_INT_ON_NEXT;
	bool disable = state->iflags & I_DISABLE_INT_ON_NEXT;

	//fprintf(stderr, "Opcode: %d\n", opcode);

	if(likely(--state->wait))
	{
		return true;
	}

	opcode = mem_read8(state, state->registers.pc);
	handler = handlers[opcode];
	handler(state);

	if(unlikely(enable))
	{
		if(unlikely(disable))
		{
			error("somehow conflicting flags are set; expect brokenness");
		}


		state->iflags &= ~I_ENABLE_INT_ON_NEXT;
		state->iflags |= I_INTERRUPTS;
	}

	if(unlikely(disable))
	{
		state->iflags &= ~I_DISABLE_INT_ON_NEXT;
		state->iflags &= ~I_INTERRUPTS;
	}

	return true;
}
