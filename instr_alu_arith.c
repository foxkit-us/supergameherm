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
 * @brief ADD HL,DE (0x19)
 * @result HL += DE; N flag reset, H if carry from bit 11, C if overflow
 */
static inline void add_hl_de(emu_state *restrict state)
{
	add_to_hl(state, state->registers.de);
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
 * @brief DAA (0x27)
 * @result Decimal-Adjust A.
 */
static inline void daa(emu_state *restrict state)
{
	uint16_t val = *(state->registers.a);

	if(*(state->registers.f) & FLAG_N)
	{
		if(*(state->registers.f) & FLAG_H)
		{
			val = (val - 6) & 0xFF;
		}

		if(*(state->registers.f) & FLAG_C)
		{
			val -= 0x60;
		}
	}
	else
	{
		if((*(state->registers.f) & FLAG_H) || (val & 0x0F))
		{
			val += 0x06;
		}

		if((*(state->registers.f) & FLAG_C) || (val > 0x9F))
		{
			val += 0x60;
		}
	}

	*(state->registers.f) &= ~(FLAG_H | FLAG_Z);

	if(val & 0x100)
	{
		*(state->registers.f) |= FLAG_C;
	}

	if(!(*(state->registers.a) = val))
	{
		*(state->registers.f) |= FLAG_Z;
	}

	state->registers.pc++;
	state->wait = 4;
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
 * @brief ADD HL,SP (0x39)
 * @result HL += SP; N flag reset, H if carry from bit 11, C if overflow
 */
static inline void add_hl_sp(emu_state *restrict state)
{
	add_to_hl(state, state->registers.sp);

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

/*!
 * @brief ADC n (0xCE)
 * @result A += immediate (n) (+1 if C flag set)
 */
static inline void adc_imm8(emu_state *restrict state)
{
	adc_common(state, mem_read8(state, ++state->registers.pc));

	// adc_common already adds 4
	state->wait += 4;
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
 * @brief SBC n (0xDE)
 * @result A -= n (+1 if C flag set)
 */
static inline void sbc_imm8(emu_state *restrict state)
{
	sbc_common(state, mem_read8(state, ++state->registers.pc));

	// sbc_common already adds 4
	state->wait += 4;
}
