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
* @brief CCF (0x3F)
* @result C flag inverted
*/
static inline void ccf(emu_state *restrict state)
{
	*(state->registers.f) ^= ~FLAG_C;
	state->registers.pc++;

	state->wait = 4;
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
	case CB_OP_SWAP:
	{
		/* swap higher and lower nibble of register <reg> */
		uint8_t x = ((*write_to >> 1) ^ (*write_to >> 5)) & ((1U << 4) - 1);
		if((*write_to ^= ((x << 1) | (x << 5))) == 0)
		{
			*(state->registers.f) |= FLAG_Z;
		}
	}
	case CB_OP_BIT:
		/* test bit <bit_number> of register <reg> */
		if(*write_to & val)
		{
			*(state->registers.f) |= FLAG_Z;
		}
		else
		{
			*(state->registers.f) &= ~FLAG_Z;
		}
		*(state->registers.f) |= FLAG_H;
		*(state->registers.f) &= ~FLAG_N;
		break;
	}

	if(reg == CB_REG_HL)
	{
		mem_write8(state, state->registers.hl, maybe_temp);
		state->wait += 8;
	}

	state->registers.pc++;
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
 * @brief CP n (0xFE) - compare A with 8-bit immediate value
 * @result flags register modified based on result
 */
static inline void cp_imm8(emu_state *restrict state)
{
	cp_common(state, mem_read8(state, ++state->registers.pc));

	// cp_common already adds 4
	state->wait += 4;
}
