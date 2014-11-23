/*!
 * @brief RLCA (0x07)
 * @result A is rotated left; C = old bit 7
 */
static inline void rlca(emu_state *restrict state)
{
	REG_A(state) = rotl_8(REG_A(state));
	if(REG_A(state) == 0)
	{
		REG_F(state) = FLAG_Z;
	}
	else if(REG_A(state) & 0x80)
	{
		REG_F(state) = FLAG_C;
	}

	REG_PC(state)++;
	state->wait = 4;
}

/*!
 * @brief RRCA (0x0F)
 * @result A is rotated right; C = old bit 0
 */
static inline void rrca(emu_state *restrict state)
{
	REG_A(state) = rotl_8(REG_A(state));
	if(REG_A(state) == 0)
	{
		REG_F(state) = FLAG_Z;
	}
	else if(REG_A(state) & 0x80)
	{
		REG_F(state) = FLAG_C;
	}

	REG_PC(state)++;
	state->wait = 4;
}

/*!
 * @brief RLA (0x17)
 * @result A is rotated left through the carry flag
 */
static inline void rla(emu_state *restrict state)
{
	/* abusing FLAG_H as a temp var. */
	if(REG_A(state) & 0x80)
	{
		REG_F(state) = FLAG_H;
	}
	else
	{
		REG_F(state) = 0x00;
	}

	REG_A(state) <<= 1;
	REG_A(state) |= ((REG_F(state) & FLAG_C) == FLAG_C);

	if(REG_F(state) & FLAG_H)
	{
		REG_F(state) = FLAG_C;
	}

	if(REG_A(state) == 0)
	{
		REG_F(state) |= FLAG_Z;
	}

	REG_PC(state)++;
	state->wait = 4;
}

/*!
 * @brief RRA (0x1F)
 * @result A is rotated right through the carry flag
 */
static inline void rra(emu_state *restrict state)
{
	/* same as above */
	if(REG_A(state) & 0x01)
	{
		REG_F(state) = FLAG_H;
	}
	else
	{
		REG_F(state) = 0x00;
	}

	REG_A(state) >>= 1;
	REG_A(state) |= ((REG_F(state) & FLAG_C) == FLAG_C);

	if(REG_F(state) & FLAG_H)
	{
		REG_F(state) = FLAG_C;
	}

	if(REG_A(state) == 0)
	{
		REG_F(state) |= FLAG_Z;
	}

	REG_PC(state)++;
	state->wait = 4;
}

/*!
* @brief CPL (0x2F)
* @result all bits of A are negated
*/
static inline void cpl(emu_state *restrict state)
{
	REG_A(state) ^= ~(REG_A(state));
	REG_PC(state)++;

	state->wait = 4;
}

/*!
* @brief SCF (0x37)
* @result C flag set
*/
static inline void scf(emu_state *restrict state)
{
	REG_F(state) |= FLAG_C;
	REG_PC(state)++;

	state->wait = 4;
}

/*!
* @brief CCF (0x3F)
* @result C flag inverted
*/
static inline void ccf(emu_state *restrict state)
{
	REG_F(state) ^= ~FLAG_C;
	REG_PC(state)++;

	state->wait = 4;
}

static inline void and_common(emu_state *restrict state, uint8_t to_and)
{
	REG_A(state) &= to_and;

	REG_F(state) = FLAG_H;

	if(!REG_A(state))
	{
		REG_F(state) |= FLAG_Z;
	}

	REG_PC(state)++;

	state->wait = 4;
}

/*!
* @brief AND B (0xA0)
* @result A &= B; Z flag set if A is now zero
*/
static inline void and_b(emu_state *restrict state)
{
	and_common(state, REG_B(state));
}

/*!
* @brief AND C (0xA1)
* @result A &= C; Z flag set if A is now zero
*/
static inline void and_c(emu_state *restrict state)
{
	and_common(state, REG_C(state));
}

/*!
* @brief AND D (0xA2)
* @result A &= D; Z flag set if A is now zero
*/
static inline void and_d(emu_state *restrict state)
{
	and_common(state, REG_D(state));
}

/*!
* @brief AND E (0xA3)
* @result A &= E; Z flag set if A is now zero
*/
static inline void and_e(emu_state *restrict state)
{
	and_common(state, REG_E(state));
}

/*!
* @brief AND H (0xA4)
* @result A &= H; Z flag set if A is now zero
*/
static inline void and_h(emu_state *restrict state)
{
	and_common(state, REG_H(state));
}

/*!
* @brief AND L (0xA5)
* @result A &= L; Z flag set if A is now zero
*/
static inline void and_l(emu_state *restrict state)
{
	and_common(state, REG_L(state));
}

/*!
* @brief AND (HL) (0xA6)
* @result A &= contents of memory at AL; Z flag set if A is now zero
*/
static inline void and_hl(emu_state *restrict state)
{
	and_common(state, mem_read8(state, REG_HL(state)));

// and_common already adds 4
	state->wait += 4;
}

/*!
* @brief AND A (0xA7)
* @result Z flag set if A is now zero
*/
static inline void and_a(emu_state *restrict state)
{
	REG_F(state) = FLAG_H;
	if(REG_A(state) == 0)
	{
		REG_F(state) |= FLAG_Z;
	}

	REG_PC(state)++;

	state->wait = 4;
}

static inline void xor_common(emu_state *restrict state, char to_xor)
{
	REG_A(state) ^= to_xor;

	REG_F(state) = 0;
	if(REG_A(state) == 0) REG_F(state) |= FLAG_Z;

	REG_PC(state)++;

	state->wait = 4;
}

/*!
* @brief XOR B (0xA8)
* @result A ^= B; Z flag set if A is now zero
*/
static inline void xor_b(emu_state *restrict state)
{
	xor_common(state, REG_B(state));
}

/*!
* @brief XOR C (0xA9)
* @result A ^= C; Z flag set if A is now zero
*/
static inline void xor_c(emu_state *restrict state)
{
	xor_common(state, REG_C(state));
}

/*!
* @brief XOR D (0xAA)
* @result A ^= D; Z flag set if A is now zero
*/
static inline void xor_d(emu_state *restrict state)
{
	xor_common(state, REG_D(state));
}

/*!
* @brief XOR E (0xAB)
* @result A ^= E; Z flag set if A is now zero
*/
static inline void xor_e(emu_state *restrict state)
{
	xor_common(state, REG_E(state));
}

/*!
* @brief XOR H (0xAC)
* @result A ^= H; Z flag set if A is now zero
*/
static inline void xor_h(emu_state *restrict state)
{
	xor_common(state, REG_H(state));
}

/*!
* @brief XOR L (0xAD)
* @result A ^= L; Z flag set if A is now zero
*/
static inline void xor_l(emu_state *restrict state)
{
	xor_common(state, REG_L(state));
}

/*!
* @brief XOR (HL) (0xAE)
* @result A ^= contents of memory at HL; Z flag set if A is now zero
*/
static inline void xor_hl(emu_state *restrict state)
{
	xor_common(state, mem_read8(state, REG_HL(state)));

// xor_common already adds 4
	state->wait += 4;
}

/*!
* @brief XOR A (0xAF)
* @result A = 0; Z flag set
*/
static inline void xor_a(emu_state *restrict state)
{
	REG_A(state) = 0;
	REG_F(state) = FLAG_Z;
	REG_PC(state)++;

	state->wait = 4;
}

static inline void or_common(emu_state *restrict state, uint8_t to_or)
{
	REG_A(state) |= to_or;

	REG_F(state) = (REG_A(state) == 0 ? FLAG_Z : 0);

	REG_PC(state)++;

	state->wait = 4;
}

/*!
* @brief OR B (0xB0)
* @result A |= B; Z flag set if A is zero
*/
static inline void or_b(emu_state *restrict state)
{
	or_common(state, REG_B(state));
}

/*!
* @brief OR C (0xB1)
* @result A |= C; Z flag set if A is zero
*/
static inline void or_c(emu_state *restrict state)
{
	or_common(state, REG_C(state));
}

/*!
* @brief OR D (0xB2)
* @result A |= D; Z flag set if A is zero
*/
static inline void or_d(emu_state *restrict state)
{
	or_common(state, REG_D(state));
}

/*!
* @brief OR E (0xB3)
* @result A |= E; Z flag set if A is zero
*/
static inline void or_e(emu_state *restrict state)
{
	or_common(state, REG_E(state));
}

/*!
* @brief OR H (0xB4)
* @result A |= H; Z flag set if A is zero
*/
static inline void or_h(emu_state *restrict state)
{
	or_common(state, REG_H(state));
}

/*!
* @brief OR L (0xB5)
* @result A |= L; Z flag set if A is zero
*/
static inline void or_l(emu_state *restrict state)
{
	or_common(state, REG_L(state));
}

/*!
* @brief OR (HL) (0xB6)
* @result A |= contents of memory at HL; Z flag set if A is zero
*/
static inline void or_hl(emu_state *restrict state)
{
	or_common(state, mem_read8(state, REG_HL(state)));

// or_common already adds 4
	state->wait += 4;
}

/*!
* @brief OR A (0xB7)
* @result A |= A; Z flag set if A is zero
*/
static inline void or_a(emu_state *restrict state)
{
	or_common(state, REG_A(state));
}

static inline void cp_common(emu_state *restrict state, uint8_t cmp)
{
	REG_F(state) = FLAG_N;
	if(REG_A(state) == cmp)
	{
		REG_F(state) |= FLAG_Z;
	}
	else
	{
		if(REG_A(state) < cmp)
		{
			REG_F(state) |= FLAG_C;
		}
		else
		{
			REG_F(state) |= FLAG_H;
		}
	}

	REG_PC(state)++;

	state->wait = 4;
}

/*!
* @brief CP B (0xB8)
* @result flags set based on how equivalent B is to A
*/
static inline void cp_b(emu_state *restrict state)
{
	cp_common(state, REG_B(state));
}

/*!
* @brief CP C (0xB9)
* @result flags set based on how equivalent C is to A
*/
static inline void cp_c(emu_state *restrict state)
{
	cp_common(state, REG_C(state));
}

/*!
* @brief CP D (0xBA)
* @result flags set based on how equivalent D is to A
*/
static inline void cp_d(emu_state *restrict state)
{
	cp_common(state, REG_D(state));
}

/*!
* @brief CP E (0xBB)
* @result flags set based on how equivalent E is to A
*/
static inline void cp_e(emu_state *restrict state)
{
	cp_common(state, REG_E(state));
}

/*!
* @brief CP H (0xBC)
* @result flags set based on how equivalent H is to A
*/
static inline void cp_h(emu_state *restrict state)
{
	cp_common(state, REG_H(state));
}

/*!
* @brief CP L (0xBD)
* @result flags set based on how equivalent L is to A
*/
static inline void cp_l(emu_state *restrict state)
{
	cp_common(state, REG_L(state));
}

/*!
* @brief CP (HL) (0xBE)
* @result flags set based on how equivalent contents of memory at HL are to A
*/
static inline void cp_hl(emu_state *restrict state)
{
	cp_common(state, mem_read8(state, REG_HL(state)));

// cp_common already adds 4
	state->wait += 4;
}

/*!
* @brief CP A (0xBF)
* @result flags set based on how equivalent A is to A... wait.. really?
*/
static inline void cp_a(emu_state *restrict state)
{
	cp_common(state, REG_A(state));
}

/*!
* @brief CB ..
* @note this is just a dispatch function for SWAP/BIT/etc
*/
static inline void cb_dispatch(emu_state *restrict state)
{
	int8_t opcode = mem_read8(state, ++REG_PC(state));
	uint8_t *write_to;
	uint8_t maybe_temp;
	uint8_t bit_number;
	cb_regs reg = (cb_regs)(opcode & 0x7);
	cb_ops op;

	state->wait = 12;

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
		write_to = &REG_B(state);
		break;
	case CB_REG_C:
		write_to = &REG_C(state);
		break;
	case CB_REG_D:
		write_to = &REG_D(state);
		break;
	case CB_REG_E:
		write_to = &REG_E(state);
		break;
	case CB_REG_H:
		write_to = &REG_H(state);
		break;
	case CB_REG_L:
		write_to = &REG_L(state);
		break;
	case CB_REG_HL:
		write_to = &maybe_temp;
		break;
	case CB_REG_A:
		write_to = &REG_A(state);
		break;
	}

	uint8_t val = (1 << bit_number);

	if(reg == CB_REG_HL)
	{
		maybe_temp = mem_read8(state, REG_HL(state));
	}

	switch(op)
	{
	case CB_OP_RLC:
		if(*write_to & 0x80)
		{
			REG_F(state) = FLAG_C;
		}
		else
		{
			REG_F(state) = 0x00;
		}

		*write_to <<= 1;
		*write_to |= ((REG_F(state) & FLAG_C) == FLAG_C);

		if(*write_to == 0)
		{
			REG_F(state) |= FLAG_Z;
		}

		break;
	case CB_OP_RRC:
		if(*write_to & 0x01)
		{
			REG_F(state) = FLAG_C;
		}
		else
		{
			REG_F(state) = 0x00;
		}

		*write_to >>= 1;
		if(REG_F(state) & FLAG_C)
		{
			*write_to |= 0x80;
		}

		if(*write_to == 0)
		{
			REG_F(state) |= FLAG_Z;
		}

		break;
	case CB_OP_RL:
		/* abusing FLAG_H as a temp var. */
		if(*write_to & 0x80)
		{
			REG_F(state) = FLAG_H;
		}
		else
		{
			REG_F(state) = 0x00;
		}

		*write_to <<= 1;
		*write_to |= ((REG_F(state) & FLAG_C) == FLAG_C);

		if(REG_F(state) & FLAG_H)
		{
			REG_F(state) = FLAG_C;
		}

		if(*write_to == 0)
		{
			REG_F(state) |= FLAG_Z;
		}

		break;
	case CB_OP_RR:
		/* same as above */
		if(*write_to & 0x01)
		{
			REG_F(state) = FLAG_H;
		}
		else
		{
			REG_F(state) = 0x00;
		}

		*write_to >>= 1;
		*write_to |= ((REG_F(state) & FLAG_C) == FLAG_C);

		if(REG_F(state) & FLAG_H)
		{
			REG_F(state) = FLAG_C;
		}

		if(*write_to == 0)
		{
			REG_F(state) |= FLAG_Z;
		}

		break;
	case CB_OP_SLA:
		if(*write_to & 0x80)
		{
			REG_F(state) = FLAG_C;
		}
		else
		{
			REG_F(state) = 0x00;
		}

		*write_to <<= 1;

		if(*write_to == 0)
		{
			REG_F(state) |= FLAG_Z;
		}

		break;
	case CB_OP_SRA:
		if(*write_to & 0x01)
		{
			REG_F(state) = FLAG_C;
		}
		else
		{
			REG_F(state) = 0x00;
		}

		*write_to = (*write_to & 0x80) | (*write_to >> 1);

		if(*write_to == 0)
		{
			REG_F(state) |= FLAG_Z;
		}

		break;
	case CB_OP_SRL:
		if(*write_to & 0x01)
		{
			REG_F(state) = FLAG_C;
		}
		else
		{
			REG_F(state) = 0x00;
		}

		*write_to >>= 1;

		if(*write_to == 0)
		{
			REG_F(state) |= FLAG_Z;
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
	case CB_OP_SWAP:
	{
		/* swap higher and lower nibble of register <reg> */
		*write_to = swap_8(*write_to);
		if(*write_to)
		{
			REG_F(state) |= FLAG_Z;
		}
	}
	case CB_OP_BIT:
		/* test bit <bit_number> of register <reg> */
		if(*write_to & val)
		{
			REG_F(state) |= FLAG_Z;
		}
		else
		{
			REG_F(state) &= ~FLAG_Z;
		}
		REG_F(state) |= FLAG_H;
		REG_F(state) &= ~FLAG_N;
		break;
	}

	if(reg == CB_REG_HL)
	{
		mem_write8(state, REG_HL(state), maybe_temp);
		state->wait += 8;
	}

	REG_PC(state)++;
}

/*!
 * @brief AND n (0xE6)
 * @result A &= n
 */
static inline void and_imm8(emu_state *restrict state)
{
	uint8_t n = mem_read8(state, ++REG_PC(state));
	and_common(state, n);

	// and_common already adds 4
	state->wait += 4;
}

/*!
 * @brief XOR n (0xEE)
 * @result A ^= n
 */
static inline void xor_imm8(emu_state *restrict state)
{
	uint8_t n = mem_read8(state, ++REG_PC(state));
	xor_common(state, n);

	// xor_common already adds 4
	state->wait += 4;
}

/*!
 * @brief OR n (0xF6)
 * @result A |= n
 */
static inline void or_imm8(emu_state *restrict state)
{
	uint8_t n = mem_read8(state, ++REG_PC(state));
	or_common(state, n);

	// and_common already adds 4
	state->wait += 4;
}

/*!
 * @brief CP n (0xFE) - compare A with 8-bit immediate value
 * @result flags register modified based on result
 */
static inline void cp_imm8(emu_state *restrict state)
{
	cp_common(state, mem_read8(state, ++REG_PC(state)));

	// cp_common already adds 4
	state->wait += 4;
}