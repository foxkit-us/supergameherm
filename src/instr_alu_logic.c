/*!
* @brief CPL (0x2F)
* @result all bits of A are negated
*/
static inline void cpl(emu_state *restrict state, uint8_t data[] UNUSED)
{
	FLAG_SET(state, FLAG_H | FLAG_N);

	REG_A(state) ^= 0xFF;

	state->wait = 4;
}

/*!
* @brief SCF (0x37)
* @result C flag set
*/
static inline void scf(emu_state *restrict state, uint8_t data[] UNUSED)
{
	FLAG_UNSET(state, FLAG_H | FLAG_N);
	FLAG_SET(state, FLAG_C);

	state->wait = 4;
}

/*!
* @brief CCF (0x3F)
* @result C flag inverted
*/
static inline void ccf(emu_state *restrict state, uint8_t data[] UNUSED)
{
	FLAG_UNSET(state, FLAG_H | FLAG_N);
	FLAG_FLIP(state, FLAG_C);

	state->wait = 4;
}

static inline void and_common(emu_state *restrict state, uint8_t to_and)
{
	REG_A(state) &= to_and;

	FLAGS_OVERWRITE(state, FLAG_H);

	if(!REG_A(state))
	{
		FLAG_SET(state, FLAG_Z);
	}

	state->wait = 4;
}

/*!
* @brief AND B (0xA0)
* @result A &= B; Z flag set if A is now zero
*/
static inline void and_b(emu_state *restrict state, uint8_t data[] UNUSED)
{
	and_common(state, REG_B(state));
}

/*!
* @brief AND C (0xA1)
* @result A &= C; Z flag set if A is now zero
*/
static inline void and_c(emu_state *restrict state, uint8_t data[] UNUSED)
{
	and_common(state, REG_C(state));
}

/*!
* @brief AND D (0xA2)
* @result A &= D; Z flag set if A is now zero
*/
static inline void and_d(emu_state *restrict state, uint8_t data[] UNUSED)
{
	and_common(state, REG_D(state));
}

/*!
* @brief AND E (0xA3)
* @result A &= E; Z flag set if A is now zero
*/
static inline void and_e(emu_state *restrict state, uint8_t data[] UNUSED)
{
	and_common(state, REG_E(state));
}

/*!
* @brief AND H (0xA4)
* @result A &= H; Z flag set if A is now zero
*/
static inline void and_h(emu_state *restrict state, uint8_t data[] UNUSED)
{
	and_common(state, REG_H(state));
}

/*!
* @brief AND L (0xA5)
* @result A &= L; Z flag set if A is now zero
*/
static inline void and_l(emu_state *restrict state, uint8_t data[] UNUSED)
{
	and_common(state, REG_L(state));
}

/*!
* @brief AND (HL) (0xA6)
* @result A &= contents of memory at HL; Z flag set if A is now zero
*/
static inline void and_hl(emu_state *restrict state, uint8_t data[] UNUSED)
{
	and_common(state, mem_read8(state, REG_HL(state)));

	// and_common already adds 4
	state->wait += 4;
}

/*!
* @brief AND A (0xA7)
* @result Z flag set if A is now zero
*/
static inline void and_a(emu_state *restrict state, uint8_t data[] UNUSED)
{
	FLAGS_OVERWRITE(state, FLAG_H);

	if(!REG_A(state))
	{
		FLAG_SET(state, FLAG_Z);
	}

	state->wait = 4;
}

static inline void xor_common(emu_state *restrict state, char to_xor)
{
	REG_A(state) ^= to_xor;

	FLAGS_CLEAR(state);

	if(!REG_A(state))
	{
		FLAG_SET(state, FLAG_Z);
	}

	state->wait = 4;
}

/*!
* @brief XOR B (0xA8)
* @result A ^= B; Z flag set if A is now zero
*/
static inline void xor_b(emu_state *restrict state, uint8_t data[] UNUSED)
{
	xor_common(state, REG_B(state));
}

/*!
* @brief XOR C (0xA9)
* @result A ^= C; Z flag set if A is now zero
*/
static inline void xor_c(emu_state *restrict state, uint8_t data[] UNUSED)
{
	xor_common(state, REG_C(state));
}

/*!
* @brief XOR D (0xAA)
* @result A ^= D; Z flag set if A is now zero
*/
static inline void xor_d(emu_state *restrict state, uint8_t data[] UNUSED)
{
	xor_common(state, REG_D(state));
}

/*!
* @brief XOR E (0xAB)
* @result A ^= E; Z flag set if A is now zero
*/
static inline void xor_e(emu_state *restrict state, uint8_t data[] UNUSED)
{
	xor_common(state, REG_E(state));
}

/*!
* @brief XOR H (0xAC)
* @result A ^= H; Z flag set if A is now zero
*/
static inline void xor_h(emu_state *restrict state, uint8_t data[] UNUSED)
{
	xor_common(state, REG_H(state));
}

/*!
* @brief XOR L (0xAD)
* @result A ^= L; Z flag set if A is now zero
*/
static inline void xor_l(emu_state *restrict state, uint8_t data[] UNUSED)
{
	xor_common(state, REG_L(state));
}

/*!
* @brief XOR (HL) (0xAE)
* @result A ^= contents of memory at HL; Z flag set if A is now zero
*/
static inline void xor_hl(emu_state *restrict state, uint8_t data[] UNUSED)
{
	xor_common(state, mem_read8(state, REG_HL(state)));

	// xor_common already adds 4
	state->wait += 4;
}

/*!
* @brief XOR A (0xAF)
* @result A = 0; Z flag set
*/
static inline void xor_a(emu_state *restrict state, uint8_t data[] UNUSED)
{
	REG_A(state) = 0;

	FLAGS_OVERWRITE(state, FLAG_Z);

	state->wait = 4;
}

static inline void or_common(emu_state *restrict state, uint8_t to_or)
{
	REG_A(state) |= to_or;

	FLAGS_CLEAR(state);

	if(!REG_A(state))
	{
		FLAG_SET(state, FLAG_Z);
	}

	state->wait = 4;
}

/*!
* @brief OR B (0xB0)
* @result A |= B; Z flag set if A is zero
*/
static inline void or_b(emu_state *restrict state, uint8_t data[] UNUSED)
{
	or_common(state, REG_B(state));
}

/*!
* @brief OR C (0xB1)
* @result A |= C; Z flag set if A is zero
*/
static inline void or_c(emu_state *restrict state, uint8_t data[] UNUSED)
{
	or_common(state, REG_C(state));
}

/*!
* @brief OR D (0xB2)
* @result A |= D; Z flag set if A is zero
*/
static inline void or_d(emu_state *restrict state, uint8_t data[] UNUSED)
{
	or_common(state, REG_D(state));
}

/*!
* @brief OR E (0xB3)
* @result A |= E; Z flag set if A is zero
*/
static inline void or_e(emu_state *restrict state, uint8_t data[] UNUSED)
{
	or_common(state, REG_E(state));
}

/*!
* @brief OR H (0xB4)
* @result A |= H; Z flag set if A is zero
*/
static inline void or_h(emu_state *restrict state, uint8_t data[] UNUSED)
{
	or_common(state, REG_H(state));
}

/*!
* @brief OR L (0xB5)
* @result A |= L; Z flag set if A is zero
*/
static inline void or_l(emu_state *restrict state, uint8_t data[] UNUSED)
{
	or_common(state, REG_L(state));
}

/*!
* @brief OR (HL) (0xB6)
* @result A |= contents of memory at HL; Z flag set if A is zero
*/
static inline void or_hl(emu_state *restrict state, uint8_t data[] UNUSED)
{
	or_common(state, mem_read8(state, REG_HL(state)));

	// or_common already adds 4
	state->wait += 4;
}

/*!
* @brief OR A (0xB7)
* @result A |= A; Z flag set if A is zero
*/
static inline void or_a(emu_state *restrict state, uint8_t data[] UNUSED)
{
	or_common(state, REG_A(state));
}

static inline void cp_common(emu_state *restrict state, uint8_t cmp)
{
	FLAGS_OVERWRITE(state, FLAG_N);

	if(REG_A(state) == cmp)
	{
		FLAG_SET(state, FLAG_Z);
	}

	if(REG_A(state) < cmp)
	{
		FLAG_SET(state, FLAG_C);
	}

	if((REG_A(state) & 0xF) < (cmp & 0xF))
	{
		FLAG_SET(state, FLAG_H);
	}

	state->wait = 4;
}

/*!
* @brief CP B (0xB8)
* @result flags set based on how equivalent B is to A
*/
static inline void cp_b(emu_state *restrict state, uint8_t data[] UNUSED)
{
	cp_common(state, REG_B(state));
}

/*!
* @brief CP C (0xB9)
* @result flags set based on how equivalent C is to A
*/
static inline void cp_c(emu_state *restrict state, uint8_t data[] UNUSED)
{
	cp_common(state, REG_C(state));
}

/*!
* @brief CP D (0xBA)
* @result flags set based on how equivalent D is to A
*/
static inline void cp_d(emu_state *restrict state, uint8_t data[] UNUSED)
{
	cp_common(state, REG_D(state));
}

/*!
* @brief CP E (0xBB)
* @result flags set based on how equivalent E is to A
*/
static inline void cp_e(emu_state *restrict state, uint8_t data[] UNUSED)
{
	cp_common(state, REG_E(state));
}

/*!
* @brief CP H (0xBC)
* @result flags set based on how equivalent H is to A
*/
static inline void cp_h(emu_state *restrict state, uint8_t data[] UNUSED)
{
	cp_common(state, REG_H(state));
}

/*!
* @brief CP L (0xBD)
* @result flags set based on how equivalent L is to A
*/
static inline void cp_l(emu_state *restrict state, uint8_t data[] UNUSED)
{
	cp_common(state, REG_L(state));
}

/*!
* @brief CP (HL) (0xBE)
* @result flags set based on how equivalent contents of memory at HL are to A
*/
static inline void cp_hl(emu_state *restrict state, uint8_t data[] UNUSED)
{
	cp_common(state, mem_read8(state, REG_HL(state)));

	// cp_common already adds 4
	state->wait += 4;
}

/*!
* @brief CP A (0xBF)
* @result flags set based on how equivalent A is to A... wait.. really?
*/
static inline void cp_a(emu_state *restrict state, uint8_t data[] UNUSED)
{
	cp_common(state, REG_A(state));
}

/*!
* @brief CB ..
* @note this is just a dispatch function for SWAP/BIT/etc
*/
static inline void cb_dispatch(emu_state *restrict state, uint8_t data[] UNUSED)
{
	uint8_t opcode = data[0];
	uint8_t *write_to = NULL;
	uint8_t maybe_temp = 0;
	uint8_t val = 0;
	cb_regs reg = (cb_regs)(opcode & 0x7);
	cb_ops op;

	state->wait = 12;

	if(opcode >= 0x40)
	{
		const uint8_t bit_number = (opcode & 0x38) >> 3;
		val = (1 << bit_number);
		op = (cb_ops)(((opcode & 0xC0) >> 6) + 7);
	}
	else
	{
		val = 0;
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
		maybe_temp = mem_read8(state, REG_HL(state));
		write_to = &maybe_temp;
		break;
	case CB_REG_A:
		write_to = &REG_A(state);
		break;
	}

	switch(op)
	{
	case CB_OP_RLC:
		FLAGS_CLEAR(state);

		if(*write_to & 0x80)
		{
			FLAG_SET(state, FLAG_C);
		}

		*write_to <<= 1;
		if(IS_FLAG(state, FLAG_C))
		{
			*write_to |= 0x1;
		}

		if(*write_to == 0)
		{
			FLAG_SET(state, FLAG_Z);
		}

		break;
	case CB_OP_RRC:
		FLAGS_CLEAR(state);

		if(*write_to & 0x01)
		{
			FLAG_SET(state, FLAG_C);
		}

		*write_to >>= 1;
		if(IS_FLAG(state, FLAG_C))
		{
			*write_to |= 0x80;
		}

		if(*write_to == 0)
		{
			FLAG_SET(state, FLAG_Z);
		}

		break;
	case CB_OP_RL:
	{
		uint8_t hi = (*write_to) & 0x80;
		bool set_low = IS_FLAG(state, FLAG_C);

		FLAGS_CLEAR(state);

		*write_to <<= 1;
		if(hi)
		{
			FLAG_SET(state, FLAG_C);
		}

		if(set_low)
		{
			*write_to |= 0x01;
		}

		if(!(*write_to))
		{
			FLAG_SET(state, FLAG_Z);
		}

		break;
	}
	case CB_OP_RR:
	{
		uint8_t lo = (*write_to) & 0x01;
		bool set_high = IS_FLAG(state, FLAG_C);

		FLAGS_CLEAR(state);

		(*write_to) >>= 1;
		if(lo)
		{
			FLAG_SET(state, FLAG_C);
		}

		if(set_high)
		{
			*write_to |= 0x80;
		}

		if(!(*write_to))
		{
			FLAG_SET(state, FLAG_Z);
		}

		break;
	}
	case CB_OP_SLA:
		FLAGS_CLEAR(state);

		if(*write_to & 0x80)
		{
			FLAG_SET(state, FLAG_C);
		}

		*write_to <<= 1;

		if(*write_to == 0)
		{
			FLAG_SET(state, FLAG_Z);
		}

		break;
	case CB_OP_SRA:
		FLAGS_CLEAR(state);

		if(*write_to & 0x01)
		{
			FLAG_SET(state, FLAG_C);
		}

		*write_to = (*write_to & 0x80) | (*write_to >> 1);

		if(*write_to == 0)
		{
			FLAG_SET(state, FLAG_Z);
		}

		break;
	case CB_OP_SRL:
		FLAGS_CLEAR(state);

		if(*write_to & 0x01)
		{
			FLAG_SET(state, FLAG_C);
		}

		*write_to >>= 1;

		if(*write_to == 0)
		{
			FLAG_SET(state, FLAG_Z);
		}

		break;
	case CB_OP_RES:
		// reset bit <bit_number> of register <reg>
		*write_to &= ~val;
		break;
	case CB_OP_SET:
		// set bit <bit_number> of register <reg>
		*write_to |= val;
		break;
	case CB_OP_SWAP:
		// swap higher and lower nibble of register <reg>
		FLAGS_CLEAR(state);

		*write_to = swap_8(*write_to);
		if(!(*write_to))
		{
			FLAG_SET(state, FLAG_Z);
		}
		break;
	case CB_OP_BIT:
		// test bit <bit_number> of register <reg>
		if((*write_to & val) != val)
		{
			FLAG_SET(state, FLAG_Z);
		}
		else
		{
			FLAG_UNSET(state, FLAG_Z);
		}

		FLAG_SET(state, FLAG_H);
		FLAG_UNSET(state, FLAG_N);
		break;
	}

	if(reg == CB_REG_HL)
	{
		mem_write8(state, REG_HL(state), maybe_temp);
		state->wait += 8;
	}
}

/*!
 * @brief AND n (0xE6)
 * @result A &= n
 */
static inline void and_d8(emu_state *restrict state, uint8_t data[])
{
	uint8_t n = data[0];
	and_common(state, n);

	// and_common already adds 4
	state->wait += 4;
}

/*!
 * @brief XOR n (0xEE)
 * @result A ^= n
 */
static inline void xor_d8(emu_state *restrict state, uint8_t data[])
{
	uint8_t n = data[0];
	xor_common(state, n);

	// xor_common already adds 4
	state->wait += 4;
}

/*!
 * @brief OR n (0xF6)
 * @result A |= n
 */
static inline void or_d8(emu_state *restrict state, uint8_t data[])
{
	uint8_t n = data[0];
	or_common(state, n);

	// and_common already adds 4
	state->wait += 4;
}

/*!
 * @brief CP n (0xFE) - compare A with 8-bit immediate value
 * @result flags register modified based on result
 */
static inline void cp_d8(emu_state *restrict state, uint8_t data[])
{
	cp_common(state, data[0]);

	// cp_common already adds 4
	state->wait += 4;
}

