#include <stdbool.h>	// bool
#include <stdint.h>	// integer types
#include <stdlib.h>	// NULL

#include "memory.h"	// mem_[read|write][8|16]
#include "params.h"	// system_types
#include "print.h"	// fatal
#include "sgherm.h"	// emulator_state, REG_*, etc


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

void dump_flags(emulator_state *state)
{
	debug("flags = %s%s%s%s",
	      (state->flag_reg & FLAG_Z) ? "Z":"z",
	      (state->flag_reg & FLAG_N) ? "N":"n",
	      (state->flag_reg & FLAG_H) ? "H":"h",
	      (state->flag_reg & FLAG_C) ? "C":"c");
}

uint8_t int_flag_read(emulator_state *state, uint16_t location)
{
	return state->memory[location];
}

void int_flag_write(emulator_state *state, uint16_t location, uint8_t data)
{
	/* only allow setting of the first five bits. */
	state->memory[location] = data & 0x1F;
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
 * @brief LD BC,nn (0x01)
 * @result BC = nn
 */
void ld_bc_imm16(emulator_state *state)
{
	uint8_t lsb = mem_read8(state, ++state->pc);
	uint8_t msb = mem_read8(state, ++state->pc);

	state->bc = (msb<<8)|lsb;

	state->pc++;
}

/*!
 * @brief INC BC (0x03)
 * @result 1 is added to BC (possibly wrapping)
 */
void inc_bc(emulator_state *state)
{
	state->bc++;
	state->pc++;
}

static inline void inc_r8(emulator_state *state, uint8_t *reg)
{
	uint8_t old = *reg;

	if(*reg ^ 0x0F) state->flag_reg &= ~FLAG_H;
	else state->flag_reg |= FLAG_H;

	*reg += 1;

	if(*reg == 0) state->flag_reg |= FLAG_Z;

	state->flag_reg &= ~FLAG_N;

	state->pc++;
}

/*!
 * @brief INC B (0x04)
 * @result 1 is added to B; Z if B is now zero, H if bit 3 overflow
 */
void inc_b(emulator_state *state)
{
	inc_r8(state, REG_B(state));
}

static inline void dec_r8(emulator_state *state, uint8_t *reg)
{
	uint8_t old = *reg;

	if(*reg & 0x0F) state->flag_reg &= ~FLAG_H;
	else state->flag_reg |= FLAG_H;

	*reg -= 1;

	if(*reg == 0) state->flag_reg |= FLAG_Z;

	state->flag_reg |= FLAG_N;

	state->pc++;
}

/*!
 * @brief DEC B (0x05)
 * @result 1 is subtracted from B; Z if B is now zero, H if bit 4 underflow
 */
void dec_b(emulator_state *state)
{
	dec_r8(state, REG_B(state));
}

/*!
 * @brief LD B,n (0x06)
 * @result B = n
 */
void ld_b_imm8(emulator_state *state)
{
	*REG_B(state) = mem_read8(state, ++state->pc);
	state->pc++;
}

/*!
 * @brief DEC BC (0x0B)
 * @result 1 is subtracted from BC (possibly wrapping)
 */
void dec_bc(emulator_state *state)
{
	state->bc--;
	state->pc++;
}

/*!
 * @brief INC C (0x0C)
 * @result 1 is added to C; Z if C is now zero, H if bit 3 overflow
 */
void inc_c(emulator_state *state)
{
	inc_r8(state, REG_C(state));
}

/*!
 * @brief DEC C (0x0C)
 * @result 1 is subtracted from C; Z if C is now zero, H if bit 4 underflow
 */
void dec_c(emulator_state *state)
{
	dec_r8(state, REG_C(state));
}

/*!
 * @brief LD C,n (0x0E)
 * @result C = n
 */
void ld_c_imm8(emulator_state *state)
{
	*REG_C(state) = mem_read8(state, ++state->pc);
	state->pc++;
}

/*!
 * @brief LD DE,nn (0x11)
 * @result DE = nn
 */
void ld_de_imm16(emulator_state *state)
{
	uint8_t lsb = mem_read8(state, ++state->pc);
	uint8_t msb = mem_read8(state, ++state->pc);

	state->de = (msb<<8)|lsb;

	state->pc++;
}

/*!
 * @brief INC DE (0x13)
 * @result 1 is added to DE (possibly wrapping)
 */
void inc_de(emulator_state *state)
{
	state->de++;
	state->pc++;
}

/*!
 * @brief INC D (0x14)
 * @result 1 is added to D; Z if D is now zero, H if bit 3 overflow
 */
void inc_d(emulator_state *state)
{
	inc_r8(state, REG_D(state));
}

/*!
 * @brief DEC D (0x15)
 * @result 1 is subtracted from D; Z if D is now zero, H if bit 4 underflow
 */
void dec_d(emulator_state *state)
{
	dec_r8(state, REG_D(state));
}

/*!
 * @brief LD D,n (0x16)
 * @result D = n
 */
void ld_d_imm8(emulator_state *state)
{
	*REG_D(state) = mem_read8(state, ++state->pc);
	state->pc++;
}

/*!
 * @brief JR n (0x18)
 * @result add n to pc
 */
void jr_imm8(emulator_state *state)
{
	int8_t to_add = mem_read8(state, ++state->pc);

	state->pc += to_add + 1;
}

/*!
 * @brief DEC DE (0x1B)
 * @result 1 is subtracted from DE (possibly wrapping)
 */
void dec_de(emulator_state *state)
{
	state->de--;
	state->pc++;
}

/*!
 * @brief INC E (0x1C)
 * @result 1 is added to E; Z if E is now zero, H if bit 3 overflow
 */
void inc_e(emulator_state *state)
{
	inc_r8(state, REG_E(state));
}

/*!
 * @brief DEC E (0x1D)
 * @result 1 is subtracted from E; Z if E is now zero, H if bit 4 underflow
 */
void dec_e(emulator_state *state)
{
	dec_r8(state, REG_E(state));
}

/*!
 * @brief LD E,n (0x1E)
 * @result E = n
 */
void ld_e_imm8(emulator_state *state)
{
	*REG_E(state) = mem_read8(state, ++state->pc);
	state->pc++;
}

/*!
 * @brief JR NZ,n (0x20)
 * @result add n to pc if Z (zero) flag clear
 */
void jr_nz_imm8(emulator_state *state)
{
	int8_t to_add = mem_read8(state, ++state->pc) + 1;

	state->pc += (state->flag_reg & FLAG_Z) ? 1 : to_add;
}

/*!
 * @brief LD HL,nn (0x21)
 * @result HL = nn
 */
void ld_hl_imm16(emulator_state *state)
{
	uint8_t lsb = mem_read8(state, ++state->pc);
	uint8_t msb = mem_read8(state, ++state->pc);

	state->hl = (msb<<8)|lsb;

	state->pc++;
}

/*!
 * @brief LD (HL+),A (0x22)
 * @result contents of memory at HL = A; HL incremented 1
 */
void ldi_hl_a(emulator_state *state)
{
	mem_write8(state, state->hl++, *REG_A(state));
	state->pc++;
}

/*!
* @brief INC HL (0x23)
* @result 1 is added to HL (possibly wrapping)
*/
void inc_hl(emulator_state *state)
{
	state->hl++;
	state->pc++;
}

/*!
 * @brief INC H (0x24)
 * @result 1 is added to H; Z if H is now zero, H if bit 3 overflow
 */
void inc_h(emulator_state *state)
{
	inc_r8(state, REG_H(state));
}

/*!
 * @brief DEC H (0x25)
 * @result 1 is subtracted from H; Z if H is now zero, H if bit 4 underflow
 */
void dec_h(emulator_state *state)
{
	dec_r8(state, REG_H(state));
}

/*!
 * @brief LD H,n (0x26)
 * @result H = n
 */
void ld_h_imm8(emulator_state *state)
{
	*REG_H(state) = mem_read8(state, ++state->pc);
	state->pc++;
}

/*!
 * @brief JR Z,n (0x28)
 * @result add n to pc if Z (zero) flag set
 */
void jr_z_imm8(emulator_state *state)
{
	int8_t to_add = mem_read8(state, ++state->pc) + 1;

	state->pc += (state->flag_reg & FLAG_Z) ? to_add : 1;
}

/*!
 * @brief LD A,(HL+) (0x2A)
 * @result A = contents of memory at HL; HL incremented 1
 */
void ldi_a_hl(emulator_state *state)
{
	*REG_A(state) = mem_read8(state, state->hl++);
	state->pc++;
}

/*!
 * @brief DEC HL (0x2B)
 * @result 1 is subtracted from HL (possibly wrapping)
 */
void dec_hl(emulator_state *state)
{
	state->hl--;
	state->pc++;
}

/*!
 * @brief INC L (0x2C)
 * @result 1 is added to L; Z if L is now zero, H if bit 3 overflow
 */
void inc_l(emulator_state *state)
{
	inc_r8(state, REG_L(state));
}

/*!
 * @brief DEC L (0x2D)
 * @result 1 is subtracted from L; Z if L is now zero, H if bit 4 underflow
 */
void dec_l(emulator_state *state)
{
	dec_r8(state, REG_L(state));
}

/*!
 * @brief LD L,n (0x2E)
 * @result L = n
 */
void ld_l_imm8(emulator_state *state)
{
	*REG_L(state) = mem_read8(state, ++state->pc);
	state->pc++;
}

/*!
 * @brief LD SP,nn (0x31)
 * @result SP = nn
 */
void ld_sp_imm16(emulator_state *state)
{
	uint8_t lsb = mem_read8(state, ++state->pc);
	uint8_t msb = mem_read8(state, ++state->pc);

	state->sp = (msb<<8)|lsb;

	state->pc++;
}

/*!
 * @brief LD (HL-),A (0x32)
 * @result contents of memory at HL = A; HL decremented 1
 */
void ldd_hl_a(emulator_state *state)
{
	mem_write8(state, state->hl--, *REG_A(state));
	state->pc++;
}

/*!
 * @brief INC SP (0x33)
 * @result 1 is added to SP (possibly wrapping)
 */
void inc_sp(emulator_state *state)
{
	state->sp++;
	state->pc++;
}

/*!
 * @brief LD (HL),n (0x36)
 * @result contents of memory at HL = n
 */
void ld_hl_imm8(emulator_state *state)
{
	uint8_t n = mem_read8(state, ++state->pc);
	mem_write8(state, state->hl, n);
	state->pc++;
}

/*!
 * @brief LD A,(HL-) (0x2A)
 * @result A = contents of memory at HL; HL decremented 1
 */
void ldd_a_hl(emulator_state *state)
{
	*REG_A(state) = mem_read8(state, state->hl--);
	state->pc++;
}

/*!
 * @brief DEC SP (0x3B)
 * @result 1 is subtracted from SP (possibly wrapping)
 */
void dec_sp(emulator_state *state)
{
	state->sp--;
	state->pc++;
}

/*!
 * @brief INC A (0x3C)
 * @result 1 is added to A; Z if A is now zero, H if bit 3 overflow
 */
void inc_a(emulator_state *state)
{
	inc_r8(state, REG_A(state));
}

/*!
 * @brief DEC A (0x3D)
 * @result 1 is subtracted from A; Z if A is now zero, H if bit 4 underflow
 */
void dec_a(emulator_state *state)
{
	dec_r8(state, REG_A(state));
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
 * @brief LD B,B (0x40)
 * @result B = B
 */
void ld_b_b(emulator_state *state)
{
	state->pc++;
}

/*!
 * @brief LD B,C (0x41)
 * @result B = C
 */
void ld_b_c(emulator_state *state)
{
	*REG_B(state) = *REG_C(state);
	state->pc++;
}

/*!
 * @brief LD B,D (0x42)
 * @result B = D
 */
void ld_b_d(emulator_state *state)
{
	*REG_B(state) = *REG_D(state);
	state->pc++;
}

/*!
 * @brief LD B,E (0x43)
 * @result B = E
 */
void ld_b_e(emulator_state *state)
{
	*REG_B(state) = *REG_E(state);
	state->pc++;
}

/*!
 * @brief LD B,H (0x44)
 * @result B = H
 */
void ld_b_h(emulator_state *state)
{
	*REG_B(state) = *REG_H(state);
	state->pc++;
}

/*!
 * @brief LD B,L (0x45)
 * @result B = L
 */
void ld_b_l(emulator_state *state)
{
	*REG_B(state) = *REG_L(state);
	state->pc++;
}

/*!
 * @brief LD B,(HL) (0x46)
 * @result B = contents of memory at HL
 */
void ld_b_hl(emulator_state *state)
{
	*REG_B(state) = mem_read8(state, state->hl);
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
 * @brief LD C,B (0x48)
 * @result C = B
 */
void ld_c_b(emulator_state *state)
{
	*REG_C(state) = *REG_B(state);
	state->pc++;
}

/*!
 * @brief LD C,C (0x49)
 * @result C = C
 */
void ld_c_c(emulator_state *state)
{
	state->pc++;
}

/*!
 * @brief LD C,D (0x4A)
 * @result C = D
 */
void ld_c_d(emulator_state *state)
{
	*REG_C(state) = *REG_D(state);
	state->pc++;
}

/*!
 * @brief LD C,E (0x4B)
 * @result C = E
 */
void ld_c_e(emulator_state *state)
{
	*REG_C(state) = *REG_E(state);
	state->pc++;
}

/*!
 * @brief LD C,H (0x4C)
 * @result C = H
 */
void ld_c_h(emulator_state *state)
{
	*REG_C(state) = *REG_H(state);
	state->pc++;
}

/*!
 * @brief LD C,L (0x4D)
 * @result C = L
 */
void ld_c_l(emulator_state *state)
{
	*REG_C(state) = *REG_L(state);
	state->pc++;
}

/*!
 * @brief LD C,(HL) (0x4E)
 * @result C = contents of memory at HL
 */
void ld_c_hl(emulator_state *state)
{
	*REG_C(state) = mem_read8(state, state->hl);
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
 * @brief LD D,B (0x50)
 * @result D = B
 */
void ld_d_b(emulator_state *state)
{
	*REG_D(state) = *REG_B(state);
	state->pc++;
}

/*!
 * @brief LD D,C (0x51)
 * @result D = C
 */
void ld_d_c(emulator_state *state)
{
	*REG_D(state) = *REG_C(state);
	state->pc++;
}

/*!
 * @brief LD D,D (0x52)
 * @result D = D
 */
void ld_d_d(emulator_state *state)
{
	state->pc++;
}

/*!
 * @brief LD D,E (0x53)
 * @result D = E
 */
void ld_d_e(emulator_state *state)
{
	*REG_D(state) = *REG_E(state);
	state->pc++;
}

/*!
 * @brief LD D,H (0x54)
 * @result D = H
 */
void ld_d_h(emulator_state *state)
{
	*REG_D(state) = *REG_H(state);
	state->pc++;
}

/*!
 * @brief LD D,L (0x55)
 * @result D = L
 */
void ld_d_l(emulator_state *state)
{
	*REG_D(state) = *REG_L(state);
	state->pc++;
}

/*!
 * @brief LD D,(HL) (0x56)
 * @result D = contents of memory at HL
 */
void ld_d_hl(emulator_state *state)
{
	*REG_D(state) = mem_read8(state, state->hl);
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
 * @brief LD E,B (0x58)
 * @result E = B
 */
void ld_e_b(emulator_state *state)
{
	*REG_E(state) = *REG_B(state);
	state->pc++;
}

/*!
 * @brief LD E,C (0x59)
 * @result E = C
 */
void ld_e_c(emulator_state *state)
{
	*REG_E(state) = *REG_C(state);
	state->pc++;
}

/*!
 * @brief LD E,D (0x5A)
 * @result E = D
 */
void ld_e_d(emulator_state *state)
{
	*REG_E(state) = *REG_D(state);
	state->pc++;
}

/*!
 * @brief LD E,E (0x5B)
 * @result E = E
 */
void ld_e_e(emulator_state *state)
{
	state->pc++;
}

/*!
 * @brief LD E,H (0x5C)
 * @result E = H
 */
void ld_e_h(emulator_state *state)
{
	*REG_E(state) = *REG_H(state);
	state->pc++;
}

/*!
 * @brief LD E,L (0x5D)
 * @result E = L
 */
void ld_e_l(emulator_state *state)
{
	*REG_E(state) = *REG_L(state);
	state->pc++;
}

/*!
 * @brief LD E,(HL) (0x5E)
 * @result E = contents of memory at HL
 */
void ld_e_hl(emulator_state *state)
{
	*REG_E(state) = mem_read8(state, state->hl);
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
 * @brief LD H,B (0x60)
 * @result H = B
 */
void ld_h_b(emulator_state *state)
{
	*REG_H(state) = *REG_B(state);
	state->pc++;
}

/*!
 * @brief LD H,C (0x61)
 * @result H = C
 */
void ld_h_c(emulator_state *state)
{
	*REG_H(state) = *REG_C(state);
	state->pc++;
}

/*!
 * @brief LD H,D (0x62)
 * @result H = D
 */
void ld_h_d(emulator_state *state)
{
	*REG_H(state) = *REG_D(state);
	state->pc++;
}

/*!
 * @brief LD H,E (0x63)
 * @result H = E
 */
void ld_h_e(emulator_state *state)
{
	*REG_H(state) = *REG_E(state);
	state->pc++;
}

/*!
 * @brief LD H,H (0x64)
 * @result H = H
 */
void ld_h_h(emulator_state *state)
{
	state->pc++;
}

/*!
 * @brief LD H,L (0x65)
 * @result H = L
 */
void ld_h_l(emulator_state *state)
{
	*REG_H(state) = *REG_L(state);
	state->pc++;
}

/*!
 * @brief LD H,(HL) (0x66)
 * @result H = contents of memory at HL
 */
void ld_h_hl(emulator_state *state)
{
	*REG_H(state) = mem_read8(state, state->hl);
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
 * @brief LD L,B (0x68)
 * @result L = B
 */
void ld_l_b(emulator_state *state)
{
	*REG_L(state) = *REG_B(state);
	state->pc++;
}

/*!
 * @brief LD L,C (0x69)
 * @result L = C
 */
void ld_l_c(emulator_state *state)
{
	*REG_L(state) = *REG_C(state);
	state->pc++;
}

/*!
 * @brief LD L,D (0x6A)
 * @result L = D
 */
void ld_l_d(emulator_state *state)
{
	*REG_L(state) = *REG_D(state);
	state->pc++;
}

/*!
 * @brief LD L,E (0x6B)
 * @result L = E
 */
void ld_l_e(emulator_state *state)
{
	*REG_L(state) = *REG_E(state);
	state->pc++;
}

/*!
 * @brief LD L,H (0x6C)
 * @result L = H
 */
void ld_l_h(emulator_state *state)
{
	*REG_L(state) = *REG_H(state);
	state->pc++;
}

/*!
 * @brief LD L,L (0x6D)
 * @result L = L
 */
void ld_l_l(emulator_state *state)
{
	state->pc++;
}

/*!
 * @brief LD L,(HL) (0x6E)
 * @result L = contents of memory at HL
 */
void ld_l_hl(emulator_state *state)
{
	*REG_L(state) = mem_read8(state, state->hl);
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

static inline void and_common(emulator_state *state, uint8_t to_and)
{
	*REG_A(state) &= to_and;

	state->flag_reg = FLAG_H;
	if(*REG_A(state) == 0) state->flag_reg |= FLAG_Z;

	state->pc++;
}

/*!
 * @brief LD (HL),B (0x70)
 * @result contents of memory at HL = B
 */
void ld_hl_b(emulator_state *state)
{
	mem_write8(state, state->hl, *REG_B(state));
	state->pc++;
}

/*!
 * @brief LD (HL),C (0x71)
 * @result contents of memory at HL = C
 */
void ld_hl_c(emulator_state *state)
{
	mem_write8(state, state->hl, *REG_C(state));
	state->pc++;
}

/*!
 * @brief LD (HL),D (0x72)
 * @result contents of memory at HL = D
 */
void ld_hl_d(emulator_state *state)
{
	mem_write8(state, state->hl, *REG_D(state));
	state->pc++;
}

/*!
 * @brief LD (HL),E (0x73)
 * @result contents of memory at HL = E
 */
void ld_hl_e(emulator_state *state)
{
	mem_write8(state, state->hl, *REG_E(state));
	state->pc++;
}

/*!
 * @brief LD (HL),H (0x74)
 * @result contents of memory at HL = H
 */
void ld_hl_h(emulator_state *state)
{
	mem_write8(state, state->hl, *REG_H(state));
	state->pc++;
}

/*!
 * @brief LD (HL),L (0x75)
 * @result contents of memory at HL = L
 */
void ld_hl_l(emulator_state *state)
{
	mem_write8(state, state->hl, *REG_L(state));
	state->pc++;
}

/*!
 * @brief LD (HL),A (0x77)
 * @result contents of memory at HL = A
 */
void ld_hl_a(emulator_state *state)
{
	mem_write8(state, state->hl, *REG_A(state));
	state->pc++;
}

/*!
 * @brief LD A,B (0x78)
 * @result A = B
 */
void ld_a_b(emulator_state *state)
{
	*REG_A(state) = *REG_B(state);
	state->pc++;
}

/*!
 * @brief LD A,C (0x79)
 * @result A = C
 */
void ld_a_c(emulator_state *state)
{
	*REG_A(state) = *REG_C(state);
	state->pc++;
}

/*!
 * @brief LD A,D (0x7A)
 * @result A = D
 */
void ld_a_d(emulator_state *state)
{
	*REG_A(state) = *REG_D(state);
	state->pc++;
}

/*!
 * @brief LD A,E (0x7B)
 * @result A = E
 */
void ld_a_e(emulator_state *state)
{
	*REG_A(state) = *REG_E(state);
	state->pc++;
}

/*!
 * @brief LD A,H (0x7C)
 * @result A = H
 */
void ld_a_h(emulator_state *state)
{
	*REG_A(state) = *REG_H(state);
	state->pc++;
}

/*!
 * @brief LD A,L (0x7D)
 * @result A = L
 */
void ld_a_l(emulator_state *state)
{
	*REG_A(state) = *REG_L(state);
	state->pc++;
}

/*!
 * @brief LD A,(HL) (0x7E)
 * @result A = contents of memory at HL
 */
void ld_a_hl(emulator_state *state)
{
	*REG_A(state) = mem_read8(state, state->hl);
	state->pc++;
}

/*!
 * @brief LD A,A (0x7F)
 * @result A = A
 */
void ld_a_a(emulator_state *state)
{
	state->pc++;
}

/*!
 * @brief AND B (0xA0)
 * @result A &= B; Z flag set if A is now zero
 */
void and_b(emulator_state *state)
{
	and_common(state, *REG_B(state));
}

/*!
 * @brief AND C (0xA1)
 * @result A &= C; Z flag set if A is now zero
 */
void and_c(emulator_state *state)
{
	and_common(state, *REG_C(state));
}

/*!
 * @brief AND D (0xA2)
 * @result A &= D; Z flag set if A is now zero
 */
void and_d(emulator_state *state)
{
	and_common(state, *REG_D(state));
}

/*!
 * @brief AND E (0xA3)
 * @result A &= E; Z flag set if A is now zero
 */
void and_e(emulator_state *state)
{
	and_common(state, *REG_E(state));
}

/*!
 * @brief AND H (0xA4)
 * @result A &= H; Z flag set if A is now zero
 */
void and_h(emulator_state *state)
{
	and_common(state, *REG_H(state));
}

/*!
 * @brief AND L (0xA5)
 * @result A &= L; Z flag set if A is now zero
 */
void and_l(emulator_state *state)
{
	and_common(state, *REG_L(state));
}

/*!
 * @brief AND (HL) (0xA6)
 * @result A &= contents of memory at AL; Z flag set if A is now zero
 */
void and_hl(emulator_state *state)
{
	and_common(state, mem_read8(state, state->hl));
}

/*!
 * @brief AND A (0xA7)
 * @result Z flag set if A is now zero
 */
void and_a(emulator_state *state)
{
	state->flag_reg = FLAG_H;
	if(*REG_A(state) == 0) state->flag_reg |= FLAG_Z;
}

static inline void xor_common(emulator_state *state, char to_xor)
{
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
 * @result A ^= L; Z flag set if A is now zero
 */
void xor_l(emulator_state *state)
{
	xor_common(state, *REG_L(state));
}

/*!
 * @brief XOR (HL) (0xAE)
 * @result A ^= contents of memory at HL; Z flag set if A is now zero
 */
void xor_hl(emulator_state *state)
{
	xor_common(state, mem_read8(state, state->hl));
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

static inline void or_common(emulator_state *state, uint8_t to_or)
{
	*REG_A(state) |= to_or;

	state->flag_reg = (*REG_A(state) == 0 ? FLAG_Z : 0);

	state->pc++;
}

/*!
 * @brief OR B (0xB0)
 * @result A |= B; Z flag set if A is zero
 */
void or_b(emulator_state *state)
{
	or_common(state, *REG_B(state));
}

/*!
 * @brief OR C (0xB1)
 * @result A |= C; Z flag set if A is zero
 */
void or_c(emulator_state *state)
{
	or_common(state, *REG_C(state));
}

/*!
 * @brief OR D (0xB2)
 * @result A |= D; Z flag set if A is zero
 */
void or_d(emulator_state *state)
{
	or_common(state, *REG_D(state));
}

/*!
 * @brief OR E (0xB3)
 * @result A |= E; Z flag set if A is zero
 */
void or_e(emulator_state *state)
{
	or_common(state, *REG_E(state));
}

/*!
 * @brief OR H (0xB4)
 * @result A |= H; Z flag set if A is zero
 */
void or_h(emulator_state *state)
{
	or_common(state, *REG_H(state));
}

/*!
 * @brief OR L (0xB5)
 * @result A |= L; Z flag set if A is zero
 */
void or_l(emulator_state *state)
{
	or_common(state, *REG_L(state));
}

/*!
 * @brief OR (HL) (0xB6)
 * @result A |= contents of memory at HL; Z flag set if A is zero
 */
void or_hl(emulator_state *state)
{
	or_common(state, mem_read8(state, state->hl));
}

/*!
 * @brief OR A (0xB7)
 * @result A |= A; Z flag set if A is zero
 */
void or_a(emulator_state *state)
{
	or_common(state, *REG_A(state));
}

/*!
 * @brief POP BC (0xC1)
 * @result BC = memory at SP; SP incremented 2
 */
void pop_bc(emulator_state *state)
{
	state->bc = mem_read16(state, state->sp);
	state->sp += 2;
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

/*!
 * @brief PUSH BC (0xC5)
 * @result contents of memory at SP = BC; SP decremented 2
 */
void push_bc(emulator_state *state)
{
	state->sp -= 2;
	mem_write16(state, state->sp, state->bc);
	state->pc++;
}

enum cb_regs {
	CB_REG_B = 0, CB_REG_C, CB_REG_D, CB_REG_E, CB_REG_H, CB_REG_L,
	CB_REG_HL, CB_REG_A
};

enum cb_ops {
	CB_OP_BIT = 1, CB_OP_RES = 2, CB_OP_SET = 3
};

/*!
 * @brief RET (0xC9) - return from CALL
 * @result pop two bytes from the stack and jump to that location
 */
void ret(emulator_state *state)
{
	state->pc = mem_read16(state, state->sp);
	state->sp += 2;
}

/*!
 * @brief RETNZ (0xC0) - return from CALL if Z flag not set
 * @result RET, if Z flag not set, otherwise nothing.
 */
void retnz(emulator_state *state)
{
	if(!(state->flag_reg & FLAG_Z)) ret(state);
	else state->pc++;
}

/*!
 * @brief RETZ (0xC8) - return from CALL if Z flag set
 * @result RET, if Z flag is set, otherwise nothing.
 */
void retz(emulator_state *state)
{
	if(state->flag_reg & FLAG_Z) ret(state);
	else state->pc++;
}

/*!
 * @brief CB ..
 * @note this is just a dispatch function for SWAP/BIT/etc
 */
void cb_dispatch(emulator_state *state)
{
	uint8_t opcode = mem_read8(state, ++state->pc);

	if(likely(opcode >= 0x40))
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
 * @brief RETNC (0xD0) - return from CALL if C flag not set
 * @result RET, if C flag not set, otherwise nothing.
 */
void retnc(emulator_state *state)
{
	if(!(state->flag_reg & FLAG_C)) ret(state);
	else state->pc++;
}

/*!
 * @brief POP DE (0xD1)
 * @result DE = memory at SP; SP incremented 2
 */
void pop_de(emulator_state *state)
{
	state->de = mem_read16(state, state->sp);
	state->sp += 2;
	state->pc++;
}

/*!
 * @brief PUSH DE (0xD5)
 * @result contents of memory at SP = DE; SP decremented 2
 */
void push_de(emulator_state *state)
{
	state->sp -= 2;
	mem_write16(state, state->sp, state->de);
	state->pc++;
}

/*!
 * @brief RETC (0xD8) - return from CALL if C flag set
 * @result RET, if C flag is set, otherwise nothing.
 */
void retc(emulator_state *state)
{
	if(state->flag_reg & FLAG_C) ret(state);
	else state->pc++;
}

/*!
 * @brief RETI (0xD9) - return from CALL and enable interrupts
 * @result RET + EI
 */
void reti(emulator_state *state)
{
	state->toggle_int_on_next = true;
	ret(state);
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
 * @brief POP HL (0xE1)
 * @result HL = memory at SP; SP incremented 2
 */
void pop_hl(emulator_state *state)
{
	state->hl = mem_read16(state, state->sp);
	state->sp += 2;
	state->pc++;
}

/*!
 * @brief LD (C),A (0xE2)
 * @result contents of memory at 0xFF00 + C = A
 */
void ld_ff00_c_a(emulator_state *state)
{
	mem_write8(state, 0xFF00 + *REG_C(state), *REG_A(state));
	state->pc++;
}

/*!
 * @brief PUSH HL (0xE5)
 * @result contents of memory at SP = HL; SP decremented 2
 */
void push_hl(emulator_state *state)
{
	state->sp -= 2;
	mem_write16(state, state->sp, state->hl);
	state->pc++;
}

/*!
 * @brief AND nn (0xE6)
 * @result A &= nn
 */
void and_imm8(emulator_state *state)
{
	uint8_t nn = mem_read8(state, ++state->pc);
	and_common(state, nn);
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
 * @brief POP AF (0xF1)
 * @result AF = memory at SP; SP incremented 2
 */
void pop_af(emulator_state *state)
{
	state->af = mem_read16(state, state->sp);
	state->sp += 2;
	state->pc++;
}

/*!
 * @brief LD A,(C) (0xF2)
 * @result A = contents of memory at 0xFF00 + C
 */
void ld_a_ff00_c(emulator_state *state)
{
	*REG_A(state) = mem_read8(state, 0xFF00 + *REG_C(state));
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
 * @brief PUSH AF (0xF5)
 * @result contents of memory at SP = AF; SP decremented 2
 */
void push_af(emulator_state *state)
{
	state->sp -= 2;
	mem_write16(state, state->sp, state->af);
	state->pc++;
}

/*!
 * @brief LD A,(nn) (0xFA) - write *nn to A
 * @result A will contain the value of memory at address nn
 */
void ld_a_d16(emulator_state *state)
{
	uint8_t lsb = mem_read8(state, ++state->pc);
	uint8_t msb = mem_read8(state, ++state->pc);

	uint16_t loc = (msb<<8) | lsb;

	*REG_A(state) = mem_read8(state, loc);

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
	/*debug("flags = %s%s%s%s; cmp = %d; A = %d",
	       (state->flag_reg & FLAG_Z) ? "Z":"z",
	       (state->flag_reg & FLAG_N) ? "N":"n",
	       (state->flag_reg & FLAG_H) ? "H":"h",
	       (state->flag_reg & FLAG_C) ? "C":"c", cmp, *REG_A(state));*/
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
	/* dump_flags(state); */

	state->pc++;
}

typedef void (*opcode_t)(emulator_state *state);

opcode_t handlers[0x100] = {
	/* 0x00 */ nop, ld_bc_imm16, NULL, inc_bc, inc_b, dec_b, ld_b_imm8, NULL,
	/* 0x08 */ NULL, NULL, NULL, dec_bc, inc_c, dec_c, ld_c_imm8, NULL,
	/* 0x10 */ NULL, ld_de_imm16, NULL, inc_de, inc_d, dec_d, ld_d_imm8, NULL,
	/* 0x18 */ jr_imm8, NULL, NULL, dec_de, inc_e, dec_e, ld_e_imm8, NULL,
	/* 0x20 */ jr_nz_imm8, ld_hl_imm16, ldi_hl_a, inc_hl, inc_h, dec_h, ld_h_imm8, NULL,
	/* 0x28 */ jr_z_imm8, NULL, ldi_a_hl, dec_hl, inc_l, dec_l, ld_l_imm8, NULL,
	/* 0x30 */ NULL, ld_sp_imm16, ldd_hl_a, inc_sp, inc_hl, dec_hl, ld_hl_imm8, NULL,
	/* 0x38 */ NULL, NULL, ldd_a_hl, dec_sp, inc_a, dec_a, ld_a_imm8, NULL,
	/* 0x40 */ ld_b_b, ld_b_c, ld_b_d, ld_b_e, ld_b_h, ld_b_l, ld_b_hl, ld_b_a,
	/* 0x48 */ ld_c_b, ld_c_c, ld_c_d, ld_c_e, ld_c_h, ld_c_l, ld_c_hl, ld_c_a,
	/* 0x50 */ ld_d_b, ld_d_c, ld_d_d, ld_d_e, ld_d_h, ld_d_l, ld_d_hl, ld_d_a,
	/* 0x58 */ ld_e_b, ld_e_c, ld_e_d, ld_e_e, ld_e_h, ld_e_l, ld_e_hl, ld_e_a,
	/* 0x60 */ ld_h_b, ld_h_c, ld_h_d, ld_h_e, ld_h_h, ld_h_l, ld_e_hl, ld_h_a,
	/* 0x68 */ ld_l_b, ld_l_c, ld_l_d, ld_l_e, ld_l_h, ld_l_l, ld_l_hl, ld_l_a,
	/* 0x70 */ ld_hl_b, ld_hl_c, ld_hl_d, ld_hl_e, ld_hl_h, ld_hl_l, NULL, ld_hl_a,
	/* 0x78 */ ld_a_b, ld_a_c, ld_a_d, ld_a_e, ld_a_h, ld_a_l, ld_a_hl, ld_a_a,
	/* 0x80 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0x88 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0x90 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0x98 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0xA0 */ and_b, and_c, and_d, and_e, and_h, and_l, and_hl, and_a,
	/* 0xA8 */ xor_b, xor_c, xor_d, xor_e, xor_h, xor_l, xor_hl, xor_a,
	/* 0xB0 */ or_b, or_c, or_d, or_e, or_h, or_l, or_hl, or_a,
	/* 0xB8 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0xC0 */ retnz, pop_bc, NULL, jp_imm16, NULL, push_bc, NULL, NULL,
	/* 0xC8 */ retz, ret, NULL, cb_dispatch, NULL, call_imm16, NULL, NULL,
	/* 0xD0 */ retnc, pop_de, NULL, NULL, NULL, push_de, NULL, NULL,
	/* 0xD8 */ retc, reti, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0xE0 */ ldh_imm8_a, pop_hl, ld_ff00_c_a, NULL, NULL, push_hl, and_imm8, NULL,
	/* 0xE8 */ NULL, NULL, ld_d16_a, NULL, NULL, NULL, NULL, NULL,
	/* 0xF0 */ ldh_a_imm8, pop_af, ld_a_ff00_c, di, NULL, push_af, NULL, NULL,
	/* 0xF8 */ NULL, NULL, ld_a_d16, ei, NULL, NULL, cp_imm8, NULL
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
void init_ctl(emulator_state *state, system_types type)
{
	state->pc = 0x0100;
	switch(type)
	{
	case SYSTEM_SGB:
		debug("Super Game Boy emulation");
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
	case SYSTEM_GB:
	default:
		debug("original Game Boy emulation");
		*REG_A(state) = 0x01;
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

	if(unlikely(handler == NULL))
	{
		fatal("invalid opcode %02X at %04X", opcode, state->pc);
	}

	WAIT_CYCLE(state, cycles[opcode], handler(state));

	if(toggle)
	{
		state->toggle_int_on_next = false;
		state->interrupts = !state->interrupts;
	}

	return true;
}
