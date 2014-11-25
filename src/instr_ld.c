/*!
 * @brief LD BC,nn (0x01)
 * @result BC = nn
 */
static inline void ld_bc_imm16(emu_state *restrict state)
{
	uint8_t lsb = mem_read8(state, ++REG_PC(state));
	uint8_t msb = mem_read8(state, ++REG_PC(state));

	REG_BC(state) = (msb<<8)|lsb;

	REG_PC(state)++;

	state->wait = 12;
}

/*!
 * @brief LD (BC),A (0x02)
 * @result contents of memory at BC = A
 */
static inline void ld_bc_a(emu_state *restrict state)
{
	mem_write8(state, REG_BC(state), REG_A(state));

	REG_PC(state)++;

	state->wait = 8;
}

/*!
 * @brief LD B,n (0x06)
 * @result B = n
 */
static inline void ld_b_imm8(emu_state *restrict state)
{
	REG_B(state) = mem_read8(state, ++REG_PC(state));
	REG_PC(state)++;

	state->wait = 8;
}

/*!
 * @brief LD (nn),SP (0x08)
 * @result contents of memory at nn = SP
 */
static inline void ld_imm16_sp(emu_state *restrict state)
{
	uint8_t lsb = mem_read8(state, ++REG_PC(state));
	uint8_t msb = mem_read8(state, ++REG_PC(state));

	uint16_t addr = (msb<<8)|lsb;

	mem_write8(state, addr, REG_SP(state));

	REG_PC(state)++;

	state->wait = 20;
}

/*!
 * @brief LD A,(BC) (0x0A)
 * @result A = contents of memory at BC
 */
static inline void ld_a_bc(emu_state *restrict state)
{
	REG_A(state) = mem_read8(state, REG_BC(state));
	REG_PC(state)++;

	state->wait = 8;
}

/*!
 * @brief LD C,n (0x0E)
 * @result C = n
 */
static inline void ld_c_imm8(emu_state *restrict state)
{
	REG_C(state) = mem_read8(state, ++REG_PC(state));
	REG_PC(state)++;

	state->wait = 8;
}

/*!
 * @brief LD DE,nn (0x11)
 * @result DE = nn
 */
static inline void ld_de_imm16(emu_state *restrict state)
{
	uint8_t lsb = mem_read8(state, ++REG_PC(state));
	uint8_t msb = mem_read8(state, ++REG_PC(state));

	REG_DE(state) = (msb<<8)|lsb;

	REG_PC(state)++;

	state->wait = 12;
}

/*!
 * @brief LD (DE),A (0x12)
 * @result contents of memory at DE = A
 */
static inline void ld_de_a(emu_state *restrict state)
{
	mem_write8(state, REG_DE(state), REG_A(state));

	REG_PC(state)++;

	state->wait = 8;
}

/*!
 * @brief LD D,n (0x16)
 * @result D = n
 */
static inline void ld_d_imm8(emu_state *restrict state)
{
	REG_D(state) = mem_read8(state, ++REG_PC(state));
	REG_PC(state)++;

	state->wait = 8;
}

/*!
 * @brief LD A,(DE) (0x1A)
 * @result A = contents of memory at (DE)
 */
static inline void ld_a_de(emu_state *restrict state)
{
	REG_A(state) = mem_read8(state, REG_DE(state));

	REG_PC(state)++;

	state->wait = 8;
}

/*!
 * @brief LD E,n (0x1E)
 * @result E = n
 */
static inline void ld_e_imm8(emu_state *restrict state)
{
	REG_E(state) = mem_read8(state, ++REG_PC(state));
	REG_PC(state)++;

	state->wait = 8;
}

/*!
 * @brief LD HL,nn (0x21)
 * @result HL = nn
 */
static inline void ld_hl_imm16(emu_state *restrict state)
{
	uint8_t lsb = mem_read8(state, ++REG_PC(state));
	uint8_t msb = mem_read8(state, ++REG_PC(state));

	REG_HL(state) = (msb<<8)|lsb;

	REG_PC(state)++;

	state->wait = 12;
}

/*!
 * @brief LD (HL+),A (0x22)
 * @result contents of memory at HL = A; HL incremented 1
 */
static inline void ldi_hl_a(emu_state *restrict state)
{
	mem_write8(state, REG_HL(state)++, REG_A(state));
	REG_PC(state)++;

	state->wait = 8;
}

/*!
 * @brief LD H,n (0x26)
 * @result H = n
 */
static inline void ld_h_imm8(emu_state *restrict state)
{
	REG_H(state) = mem_read8(state, ++REG_PC(state));
	REG_PC(state)++;

	state->wait = 8;
}

/*!
 * @brief LD A,(HL+) (0x2A)
 * @result A = contents of memory at HL; HL incremented 1
 */
static inline void ldi_a_hl(emu_state *restrict state)
{
	REG_A(state) = mem_read8(state, REG_HL(state)++);
	REG_PC(state)++;

	state->wait = 8;
}

/*!
 * @brief LD L,n (0x2E)
 * @result L = n
 */
static inline void ld_l_imm8(emu_state *restrict state)
{
	REG_L(state) = mem_read8(state, ++REG_PC(state));
	REG_PC(state)++;

	state->wait = 8;
}

/*!
 * @brief LD SP,nn (0x31)
 * @result SP = nn
 */
static inline void ld_sp_imm16(emu_state *restrict state)
{
	uint8_t lsb = mem_read8(state, ++REG_PC(state));
	uint8_t msb = mem_read8(state, ++REG_PC(state));

	REG_SP(state) = (msb<<8)|lsb;

	REG_PC(state)++;

	state->wait = 12;
}

/*!
 * @brief LD (HL-),A (0x32)
 * @result contents of memory at HL = A; HL decremented 1
 */
static inline void ldd_hl_a(emu_state *restrict state)
{
	mem_write8(state, REG_HL(state)--, REG_A(state));
	REG_PC(state)++;

	state->wait = 8;
}

/*!
 * @brief LD (HL),n (0x36)
 * @result contents of memory at HL = n
 */
static inline void ld_hl_imm8(emu_state *restrict state)
{
	uint8_t n = mem_read8(state, ++REG_PC(state));
	mem_write8(state, REG_HL(state), n);
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief LD A,(HL-) (0x3A)
 * @result A = contents of memory at HL; HL decremented 1
 */
static inline void ldd_a_hl(emu_state *restrict state)
{
	REG_A(state) = mem_read8(state, REG_HL(state)--);
	REG_PC(state)++;

	state->wait = 8;
}

/*!
 * @brief LD A,n (0x3E)
 * @result A = n
 */
static inline void ld_a_imm8(emu_state *restrict state)
{
	REG_A(state) = mem_read8(state, ++REG_PC(state));
	REG_PC(state)++;

	state->wait = 8;
}

/*!
 * @brief LD B,B (0x40)
 * @result B = B
 */
static inline void ld_b_b(emu_state *restrict state)
{
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief LD B,C (0x41)
 * @result B = C
 */
static inline void ld_b_c(emu_state *restrict state)
{
	REG_B(state) = REG_C(state);
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief LD B,D (0x42)
 * @result B = D
 */
static inline void ld_b_d(emu_state *restrict state)
{
	REG_B(state) = REG_D(state);
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief LD B,E (0x43)
 * @result B = E
 */
static inline void ld_b_e(emu_state *restrict state)
{
	REG_B(state) = REG_E(state);
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief LD B,H (0x44)
 * @result B = H
 */
static inline void ld_b_h(emu_state *restrict state)
{
	REG_B(state) = REG_H(state);
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief LD B,L (0x45)
 * @result B = L
 */
static inline void ld_b_l(emu_state *restrict state)
{
	REG_B(state) = REG_L(state);
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief LD B,(HL) (0x46)
 * @result B = contents of memory at HL
 */
static inline void ld_b_hl(emu_state *restrict state)
{
	REG_B(state) = mem_read8(state, REG_HL(state));
	REG_PC(state)++;

	state->wait = 8;
}

/*!
 * @brief LD B,A (0x47)
 * @result B = A
 */
static inline void ld_b_a(emu_state *restrict state)
{
	REG_B(state) = REG_A(state);
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief LD C,B (0x48)
 * @result C = B
 */
static inline void ld_c_b(emu_state *restrict state)
{
	REG_C(state) = REG_B(state);
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief LD C,C (0x49)
 * @result C = C
 */
static inline void ld_c_c(emu_state *restrict state)
{
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief LD C,D (0x4A)
 * @result C = D
 */
static inline void ld_c_d(emu_state *restrict state)
{
	REG_C(state) = REG_D(state);
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief LD C,E (0x4B)
 * @result C = E
 */
static inline void ld_c_e(emu_state *restrict state)
{
	REG_C(state) = REG_E(state);
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief LD C,H (0x4C)
 * @result C = H
 */
static inline void ld_c_h(emu_state *restrict state)
{
	REG_C(state) = REG_H(state);
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief LD C,L (0x4D)
 * @result C = L
 */
static inline void ld_c_l(emu_state *restrict state)
{
	REG_C(state) = REG_L(state);
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief LD C,(HL) (0x4E)
 * @result C = contents of memory at HL
 */
static inline void ld_c_hl(emu_state *restrict state)
{
	REG_C(state) = mem_read8(state, REG_HL(state));
	REG_PC(state)++;

	state->wait = 8;
}

/*!
 * @brief LD C,A (0x4F)
 * @result C = A
 */
static inline void ld_c_a(emu_state *restrict state)
{
	REG_C(state) = REG_A(state);
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief LD D,B (0x50)
 * @result D = B
 */
static inline void ld_d_b(emu_state *restrict state)
{
	REG_D(state) = REG_B(state);
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief LD D,C (0x51)
 * @result D = C
 */
static inline void ld_d_c(emu_state *restrict state)
{
	REG_D(state) = REG_C(state);
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief LD D,D (0x52)
 * @result D = D
 */
static inline void ld_d_d(emu_state *restrict state)
{
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief LD D,E (0x53)
 * @result D = E
 */
static inline void ld_d_e(emu_state *restrict state)
{
	REG_D(state) = REG_E(state);
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief LD D,H (0x54)
 * @result D = H
 */
static inline void ld_d_h(emu_state *restrict state)
{
	REG_D(state) = REG_H(state);
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief LD D,L (0x55)
 * @result D = L
 */
static inline void ld_d_l(emu_state *restrict state)
{
	REG_D(state) = REG_L(state);
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief LD D,(HL) (0x56)
 * @result D = contents of memory at HL
 */
static inline void ld_d_hl(emu_state *restrict state)
{
	REG_D(state) = mem_read8(state, REG_HL(state));
	REG_PC(state)++;

	state->wait = 8;
}

/*!
 * @brief LD D,A (0x57)
 * @result D = A
 */
static inline void ld_d_a(emu_state *restrict state)
{
	REG_D(state) = REG_A(state);
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief LD E,B (0x58)
 * @result E = B
 */
static inline void ld_e_b(emu_state *restrict state)
{
	REG_E(state) = REG_B(state);
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief LD E,C (0x59)
 * @result E = C
 */
static inline void ld_e_c(emu_state *restrict state)
{
	REG_E(state) = REG_C(state);
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief LD E,D (0x5A)
 * @result E = D
 */
static inline void ld_e_d(emu_state *restrict state)
{
	REG_E(state) = REG_D(state);
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief LD E,E (0x5B)
 * @result E = E
 */
static inline void ld_e_e(emu_state *restrict state)
{
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief LD E,H (0x5C)
 * @result E = H
 */
static inline void ld_e_h(emu_state *restrict state)
{
	REG_E(state) = REG_H(state);
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief LD E,L (0x5D)
 * @result E = L
 */
static inline void ld_e_l(emu_state *restrict state)
{
	REG_E(state) = REG_L(state);
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief LD E,(HL) (0x5E)
 * @result E = contents of memory at HL
 */
static inline void ld_e_hl(emu_state *restrict state)
{
	REG_E(state) = mem_read8(state, REG_HL(state));
	REG_PC(state)++;

	state->wait = 8;
}

/*!
 * @brief LD E,A (0x5F)
 * @result E = A
 */
static inline void ld_e_a(emu_state *restrict state)
{
	REG_E(state) = REG_A(state);
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief LD H,B (0x60)
 * @result H = B
 */
static inline void ld_h_b(emu_state *restrict state)
{
	REG_H(state) = REG_B(state);
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief LD H,C (0x61)
 * @result H = C
 */
static inline void ld_h_c(emu_state *restrict state)
{
	REG_H(state) = REG_C(state);
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief LD H,D (0x62)
 * @result H = D
 */
static inline void ld_h_d(emu_state *restrict state)
{
	REG_H(state) = REG_D(state);
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief LD H,E (0x63)
 * @result H = E
 */
static inline void ld_h_e(emu_state *restrict state)
{
	REG_H(state) = REG_E(state);
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief LD H,H (0x64)
 * @result H = H
 */
static inline void ld_h_h(emu_state *restrict state)
{
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief LD H,L (0x65)
 * @result H = L
 */
static inline void ld_h_l(emu_state *restrict state)
{
	REG_H(state) = REG_L(state);
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief LD H,(HL) (0x66)
 * @result H = contents of memory at HL
 */
static inline void ld_h_hl(emu_state *restrict state)
{
	REG_H(state) = mem_read8(state, REG_HL(state));
	REG_PC(state)++;

	state->wait = 8;
}

/*!
 * @brief LD H,A (0x67)
 * @result H = A
 */
static inline void ld_h_a(emu_state *restrict state)
{
	REG_H(state) = REG_A(state);
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief LD L,B (0x68)
 * @result L = B
 */
static inline void ld_l_b(emu_state *restrict state)
{
	REG_L(state) = REG_B(state);
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief LD L,C (0x69)
 * @result L = C
 */
static inline void ld_l_c(emu_state *restrict state)
{
	REG_L(state) = REG_C(state);
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief LD L,D (0x6A)
 * @result L = D
 */
static inline void ld_l_d(emu_state *restrict state)
{
	REG_L(state) = REG_D(state);
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief LD L,E (0x6B)
 * @result L = E
 */
static inline void ld_l_e(emu_state *restrict state)
{
	REG_L(state) = REG_E(state);
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief LD L,H (0x6C)
 * @result L = H
 */
static inline void ld_l_h(emu_state *restrict state)
{
	REG_L(state) = REG_H(state);
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief LD L,L (0x6D)
 * @result L = L
 */
static inline void ld_l_l(emu_state *restrict state)
{
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief LD L,(HL) (0x6E)
 * @result L = contents of memory at HL
 */
static inline void ld_l_hl(emu_state *restrict state)
{
	REG_L(state) = mem_read8(state, REG_HL(state));
	REG_PC(state)++;

	state->wait = 8;
}

/*!
 * @brief LD L,A (0x6F)
 * @result L = A
 */
static inline void ld_l_a(emu_state *restrict state)
{
	REG_L(state) = REG_A(state);
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief LD (HL),B (0x70)
 * @result contents of memory at HL = B
 */
static inline void ld_hl_b(emu_state *restrict state)
{
	mem_write8(state, REG_HL(state), REG_B(state));
	REG_PC(state)++;

	state->wait = 8;
}

/*!
 * @brief LD (HL),C (0x71)
 * @result contents of memory at HL = C
 */
static inline void ld_hl_c(emu_state *restrict state)
{
	mem_write8(state, REG_HL(state), REG_C(state));
	REG_PC(state)++;

	state->wait = 8;
}

/*!
 * @brief LD (HL),D (0x72)
 * @result contents of memory at HL = D
 */
static inline void ld_hl_d(emu_state *restrict state)
{
	mem_write8(state, REG_HL(state), REG_D(state));
	REG_PC(state)++;

	state->wait = 8;
}

/*!
 * @brief LD (HL),E (0x73)
 * @result contents of memory at HL = E
 */
static inline void ld_hl_e(emu_state *restrict state)
{
	mem_write8(state, REG_HL(state), REG_E(state));
	REG_PC(state)++;

	state->wait = 8;
}

/*!
 * @brief LD (HL),H (0x74)
 * @result contents of memory at HL = H
 */
static inline void ld_hl_h(emu_state *restrict state)
{
	mem_write8(state, REG_HL(state), REG_H(state));
	REG_PC(state)++;

	state->wait = 8;
}

/*!
 * @brief LD (HL),L (0x75)
 * @result contents of memory at HL = L
 */
static inline void ld_hl_l(emu_state *restrict state)
{
	mem_write8(state, REG_HL(state), REG_L(state));
	REG_PC(state)++;

	state->wait = 8;
}

/*!
 * @brief LD (HL),A (0x77)
 * @result contents of memory at HL = A
 */
static inline void ld_hl_a(emu_state *restrict state)
{
	mem_write8(state, REG_HL(state), REG_A(state));
	REG_PC(state)++;

	state->wait = 8;
}

/*!
 * @brief LD A,B (0x78)
 * @result A = B
 */
static inline void ld_a_b(emu_state *restrict state)
{
	REG_A(state) = REG_B(state);
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief LD A,C (0x79)
 * @result A = C
 */
static inline void ld_a_c(emu_state *restrict state)
{
	REG_A(state) = REG_C(state);
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief LD A,D (0x7A)
 * @result A = D
 */
static inline void ld_a_d(emu_state *restrict state)
{
	REG_A(state) = REG_D(state);
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief LD A,E (0x7B)
 * @result A = E
 */
static inline void ld_a_e(emu_state *restrict state)
{
	REG_A(state) = REG_E(state);
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief LD A,H (0x7C)
 * @result A = H
 */
static inline void ld_a_h(emu_state *restrict state)
{
	REG_A(state) = REG_H(state);
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief LD A,L (0x7D)
 * @result A = L
 */
static inline void ld_a_l(emu_state *restrict state)
{
	REG_A(state) = REG_L(state);
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief LD A,(HL) (0x7E)
 * @result A = contents of memory at HL
 */
static inline void ld_a_hl(emu_state *restrict state)
{
	REG_A(state) = mem_read8(state, REG_HL(state));
	REG_PC(state)++;

	state->wait = 8;
}

/*!
 * @brief LD A,A (0x7F)
 * @result A = A
 */
static inline void ld_a_a(emu_state *restrict state)
{
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief LDH n,A (0xE0) - write A to 0xff00+n
 * @result the I/O register n will contain the value of A
 */
static inline void ldh_imm8_a(emu_state *restrict state)
{
	uint16_t write = mem_read8(state, ++REG_PC(state));
	write += 0xFF00;

	mem_write8(state, write, REG_A(state));

	REG_PC(state)++;

	state->wait = 12;
}

/*!
 * @brief LD (C),A (0xE2)
 * @result contents of memory at 0xFF00 + C = A
 */
static inline void ld_ff00_c_a(emu_state *restrict state)
{
	mem_write8(state, 0xFF00 + REG_C(state), REG_A(state));
	REG_PC(state)++;

	state->wait = 8;
}

/*!
 * @brief LD (nn),A (0xEA) - write A to *nn
 * @result the memory at address nn will contain the value of A
 */
static inline void ld_d16_a(emu_state *restrict state)
{
	uint8_t lsb = mem_read8(state, ++REG_PC(state));
	uint8_t msb = mem_read8(state, ++REG_PC(state));

	uint16_t loc = (msb<<8) | lsb;

	mem_write8(state, loc, REG_A(state));

	REG_PC(state)++;

	state->wait = 16;
}

/*!
 * @brief LDH A,nn (0xF0) - read 0xff00+n to A
 * @result A will contain the value of the I/O register n
 */
static inline void ldh_a_imm8(emu_state *restrict state)
{
	uint8_t loc = mem_read8(state, ++REG_PC(state));
	REG_A(state) = mem_read8(state, 0xFF00 + loc);

	REG_PC(state)++;

	state->wait = 12;
}

/*!
 * @brief LD A,(C) (0xF2)
 * @result A = contents of memory at 0xFF00 + C
 */
static inline void ld_a_ff00_c(emu_state *restrict state)
{
	REG_A(state) = mem_read8(state, 0xFF00 + REG_C(state));
	REG_PC(state)++;

	state->wait = 8;
}

/*!
 * @brief LD HL,SP+n (0xF8) - add n to SP giving HL
 * @result HL = SP + n
 */
static inline void ld_hl_sp_imm8(emu_state *restrict state)
{
	/* per docs, this is a SIGNED add */
	int8_t n = mem_read8(state, ++REG_PC(state));
	uint32_t temp = REG_SP(state) + n;

	REG_HL(state) = temp;

	FLAGS_CLEAR(state);

	if(temp)
	{
		if(temp & 0x10000)
		{
			FLAG_SET(state, FLAG_C);
		}

		// Half carry
		if(((REG_PC(state) & 0x7FF) + (n & 0x7FF)) & 0x800)
		{
			FLAG_SET(state, FLAG_H);
		}
	}

	REG_PC(state)++;

	state->wait = 12;
}

/*!
 * @brief LD SP,HL (0xF9)
 * @result SP = HL
 */
static inline void ld_sp_hl(emu_state *restrict state)
{
	REG_SP(state) = REG_HL(state);

	REG_PC(state)++;

	state->wait = 8;
}

/*!
 * @brief LD A,(nn) (0xFA) - write *nn to A
 * @result A will contain the value of memory at address nn
 */
static inline void ld_a_d16(emu_state *restrict state)
{
	uint8_t lsb = mem_read8(state, ++REG_PC(state));
	uint8_t msb = mem_read8(state, ++REG_PC(state));

	uint16_t loc = (msb<<8) | lsb;

	REG_A(state) = mem_read8(state, loc);

	REG_PC(state)++;

	state->wait = 16;
}
