/*!
 * @brief INC BC (0x03)
 * @result 1 is added to BC (possibly wrapping)
 */
static inline void inc_bc(emu_state *restrict state, uint8_t data[] UNUSED)
{
	if(state->system < SYSTEM_SGB)
	{
		maybe_corrupt_oam(state, REG_BC(state));
	}
	REG_BC(state)++;

	state->wait = 8;
}

static inline void inc_r8(emu_state *restrict state, uint8_t *reg)
{
	FLAG_UNSET(state, FLAG_N);

	if(++(*reg) & 0x0F)
	{
		FLAG_UNSET(state, FLAG_H);
	}
	else
	{
		FLAG_SET(state, FLAG_H);
	}

	if(!(*reg))
	{
		FLAG_SET(state, FLAG_Z);
	}
	else
	{
		FLAG_UNSET(state, FLAG_Z);
	}

	state->wait = 4;
}

/*!
 * @brief INC B (0x04)
 * @result 1 is added to B; Z if B is now zero, H if bit 3 overflow
 */
static inline void inc_b(emu_state *restrict state, uint8_t data[] UNUSED)
{
	inc_r8(state, &REG_B(state));
}

static inline void dec_r8(emu_state *restrict state, uint8_t *reg)
{
	FLAG_SET(state, FLAG_N);

	if(!(*reg & 0x0F))
	{
		FLAG_SET(state, FLAG_H);
	}
	else
	{
		FLAG_UNSET(state, FLAG_H);
	}

	if(--(*reg))
	{
		FLAG_UNSET(state, FLAG_Z);
	}
	else
	{
		FLAG_SET(state, FLAG_Z);
	}

	state->wait = 4;
}

/*!
 * @brief DEC B (0x05)
 * @result 1 is subtracted from B; Z if B is now zero, H if bit 4 underflow
 */
static inline void dec_b(emu_state *restrict state, uint8_t data[] UNUSED)
{
	dec_r8(state, &REG_B(state));
}

static inline void add_to_hl(emu_state *restrict state, uint16_t to_add)
{
	uint32_t temp = REG_HL(state) + to_add;

	FLAG_UNSET(state, FLAG_N);

	if(temp > 0xFFFF)
	{
		FLAG_SET(state, FLAG_C);
	}
	else
	{
		FLAG_UNSET(state, FLAG_C);
	}

	if((REG_HL(state) & 0xFFF) > (temp & 0xFFF))
	{
		FLAG_SET(state, FLAG_H);
	}
	else
	{
		FLAG_UNSET(state, FLAG_H);
	}

	REG_HL(state) = (uint16_t)temp;

	state->wait = 12;
}

/*!
 * @brief RLCA (0x07)
 * @result A is rotated left; C = old bit 7
 */
static inline void rlca(emu_state *restrict state, uint8_t data[] UNUSED)
{
	FLAGS_CLEAR(state);

	REG_A(state) = rotl_8(REG_A(state), 1);

	if(REG_A(state) & 0x01)
	{
		FLAG_SET(state, FLAG_C);
	}

	state->wait = 4;
}

/*!
 * @brief ADD HL,BC (0x09)
 * @result HL += BC; N flag reset, H if carry from bit 11, C if overflow
 */
static inline void add_hl_bc(emu_state *restrict state, uint8_t data[] UNUSED)
{
	add_to_hl(state, REG_BC(state));
}

/*!
 * @brief DEC BC (0x0B)
 * @result 1 is subtracted from BC (possibly wrapping)
 */
static inline void dec_bc(emu_state *restrict state, uint8_t data[] UNUSED)
{
	if(state->system < SYSTEM_SGB)
	{
		maybe_corrupt_oam(state, REG_BC(state));
	}
	REG_BC(state)--;

	state->wait = 8;
}

/*!
 * @brief INC C (0x0C)
 * @result 1 is added to C; Z if C is now zero, H if bit 3 overflow
 */
static inline void inc_c(emu_state *restrict state, uint8_t data[] UNUSED)
{
	inc_r8(state, &REG_C(state));
}

/*!
 * @brief DEC C (0x0D)
 * @result 1 is subtracted from C; Z if C is now zero, H if bit 4 underflow
 */
static inline void dec_c(emu_state *restrict state, uint8_t data[] UNUSED)
{
	dec_r8(state, &REG_C(state));
}

/*!
 * @brief RRCA (0x0F)
 * @result A is rotated right; C = old bit 0
 */
static inline void rrca(emu_state *restrict state, uint8_t data[] UNUSED)
{
	FLAGS_CLEAR(state);

	REG_A(state) = rotr_8(REG_A(state), 1);

	if(REG_A(state) & 0x80)
	{
		FLAG_SET(state, FLAG_C);
	}

	state->wait = 4;
}

/*!
 * @brief INC DE (0x13)
 * @result 1 is added to DE (possibly wrapping)
 */
static inline void inc_de(emu_state *restrict state, uint8_t data[] UNUSED)
{
	if(state->system < SYSTEM_SGB)
	{
		maybe_corrupt_oam(state, REG_DE(state));
	}
	REG_DE(state)++;

	state->wait = 8;
}

/*!
 * @brief INC D (0x14)
 * @result 1 is added to D; Z if D is now zero, H if bit 3 overflow
 */
static inline void inc_d(emu_state *restrict state, uint8_t data[] UNUSED)
{
	inc_r8(state, &REG_D(state));
}

/*!
 * @brief DEC D (0x15)
 * @result 1 is subtracted from D; Z if D is now zero, H if bit 4 underflow
 */
static inline void dec_d(emu_state *restrict state, uint8_t data[] UNUSED)
{
	dec_r8(state, &REG_D(state));
}

/*!
 * @brief ADD HL,DE (0x19)
 * @result HL += DE; N flag reset, H if carry from bit 11, C if overflow
 */
static inline void add_hl_de(emu_state *restrict state, uint8_t data[] UNUSED)
{
	add_to_hl(state, REG_DE(state));
}

/*!
 * @brief DEC DE (0x1B)
 * @result 1 is subtracted from DE (possibly wrapping)
 */
static inline void dec_de(emu_state *restrict state, uint8_t data[] UNUSED)
{
	if(state->system < SYSTEM_SGB)
	{
		maybe_corrupt_oam(state, REG_DE(state));
	}
	REG_DE(state)--;

	state->wait = 8;
}

/*!
 * @brief INC E (0x1C)
 * @result 1 is added to E; Z if E is now zero, H if bit 3 overflow
 */
static inline void inc_e(emu_state *restrict state, uint8_t data[] UNUSED)
{
	inc_r8(state, &REG_E(state));
}

/*!
 * @brief DEC E (0x1D)
 * @result 1 is subtracted from E; Z if E is now zero, H if bit 4 underflow
 */
static inline void dec_e(emu_state *restrict state, uint8_t data[] UNUSED)
{
	dec_r8(state, &REG_E(state));
}

/*!
 * @brief RLA (0x17)
 * @result A is rotated left through the carry flag
 */
static inline void rla(emu_state *restrict state, uint8_t data[] UNUSED)
{
	uint8_t carry = (IS_FLAG(state, FLAG_C) != 0);

	FLAGS_CLEAR(state);

	if(REG_A(state) & 0x80)
	{
		FLAG_SET(state, FLAG_C);
	}

	REG_A(state) <<= 1;
	REG_A(state) |= carry;

	state->wait = 4;
}

/*!
 * @brief RRA (0x1F)
 * @result A is rotated right through the carry flag
 */
static inline void rra(emu_state *restrict state, uint8_t data[] UNUSED)
{
	uint8_t carry = ((IS_FLAG(state, FLAG_C) != 0) << 7);

	FLAGS_CLEAR(state);

	if(REG_A(state) & 0x01)
	{
		FLAG_SET(state, FLAG_C);
	}

	REG_A(state) >>= 1;
	REG_A(state) |= carry;

	state->wait = 4;
}

/*!
 * @brief INC HL (0x23)
 * @result 1 is added to HL (possibly wrapping)
 */
static inline void inc_hl(emu_state *restrict state, uint8_t data[] UNUSED)
{
	if(state->system < SYSTEM_SGB)
	{
		maybe_corrupt_oam(state, REG_HL(state));
	}
	REG_HL(state)++;

	state->wait = 8;
}

/*!
 * @brief INC H (0x24)
 * @result 1 is added to H; Z if H is now zero, H if bit 3 overflow
 */
static inline void inc_h(emu_state *restrict state, uint8_t data[] UNUSED)
{
	inc_r8(state, &REG_H(state));
}

/*!
 * @brief DEC H (0x25)
 * @result 1 is subtracted from H; Z if H is now zero, H if bit 4 underflow
 */
static inline void dec_h(emu_state *restrict state, uint8_t data[] UNUSED)
{
	dec_r8(state, &REG_H(state));
}

/*!
 * @brief DAA (0x27)
 * @result Decimal-Adjust A.
 */
static inline void daa(emu_state *restrict state, uint8_t data[] UNUSED)
{
	uint16_t val = REG_A(state);

	if(IS_FLAG(state, FLAG_N))
	{
		if(IS_FLAG(state, FLAG_H))
		{
			val = (val - 6) & 0xFF;
		}

		if(IS_FLAG(state, FLAG_C))
		{
			val -= 0x60;
		}
	}
	else
	{
		if(IS_FLAG(state, FLAG_H) || (val & 0x0F) > 0x09)
		{
			val += 0x06;
		}

		if(IS_FLAG(state, FLAG_C) || (val > 0x9F))
		{
			val += 0x60;
		}
	}

	FLAG_UNSET(state, FLAG_H | FLAG_Z);

	if(val & 0x100)
	{
		FLAG_SET(state, FLAG_C);
	}

	if(!(REG_A(state) = (uint8_t)val))
	{
		FLAG_SET(state, FLAG_Z);
	}

	state->wait = 4;
}

/*!
 * @brief ADD HL,HL (0x29)
 * @result HL += HL; N flag reset, H if carry from bit 11, C if overflow
 */
static inline void add_hl_hl(emu_state *restrict state, uint8_t data[] UNUSED)
{
	add_to_hl(state, REG_HL(state));
}

/*!
 * @brief DEC HL (0x2B)
 * @result 1 is subtracted from HL (possibly wrapping)
 */
static inline void dec_hl(emu_state *restrict state, uint8_t data[] UNUSED)
{
	if(state->system < SYSTEM_SGB)
	{
		maybe_corrupt_oam(state, REG_HL(state));
	}
	REG_HL(state)--;

	state->wait = 8;
}

/*!
 * @brief INC L (0x2C)
 * @result 1 is added to L; Z if L is now zero, H if bit 3 overflow
 */
static inline void inc_l(emu_state *restrict state, uint8_t data[] UNUSED)
{
	inc_r8(state, &REG_L(state));
}

/*!
 * @brief DEC L (0x2D)
 * @result 1 is subtracted from L; Z if L is now zero, H if bit 4 underflow
 */
static inline void dec_l(emu_state *restrict state, uint8_t data[] UNUSED)
{
	dec_r8(state, &REG_L(state));
}

/*!
 * @brief INC SP (0x33)
 * @result 1 is added to SP (possibly wrapping)
 */
static inline void inc_sp(emu_state *restrict state, uint8_t data[] UNUSED)
{
	if(state->system < SYSTEM_SGB)
	{
		maybe_corrupt_oam(state, REG_SP(state));
	}
	REG_SP(state)++;

	state->wait = 8;
}

/*!
 * @brief INC (HL) (0x34)
 * @result 1 is added to the contents of memory pointed at by HL
 */
static inline void inc_hl_mem(emu_state *restrict state, uint8_t data[] UNUSED)
{
	uint8_t val = mem_read8(state, REG_HL(state));

	FLAG_UNSET(state, FLAG_N);

	if(!(++val & 0x0F))
	{
		FLAG_SET(state, FLAG_H);
	}
	else
	{
		FLAG_UNSET(state, FLAG_H);
	}

	if(!val)
	{
		FLAG_SET(state, FLAG_Z);
	}
	else
	{
		FLAG_UNSET(state, FLAG_Z);
	}

	mem_write8(state, REG_HL(state), val);

	state->wait = 12;
}

/*!
 * @brief DEC (HL) (0x35)
 * @result 1 is added to the contents of memory pointed at by HL
 */
static inline void dec_hl_mem(emu_state *restrict state, uint8_t data[] UNUSED)
{
	uint8_t val = mem_read8(state, REG_HL(state));

	FLAG_SET(state, FLAG_N);

	if(!(val & 0x0F))
	{
		FLAG_SET(state, FLAG_H);
	}
	else
	{
		FLAG_UNSET(state, FLAG_H);
	}

	if(!(--val))
	{
		FLAG_SET(state, FLAG_Z);
	}
	else
	{
		FLAG_UNSET(state, FLAG_Z);
	}

	mem_write8(state, REG_HL(state), val);

	state->wait = 12;
}

/*!
 * @brief ADD HL,SP (0x39)
 * @result HL += SP; N flag reset, H if carry from bit 11, C if overflow
 */
static inline void add_hl_sp(emu_state *restrict state, uint8_t data[] UNUSED)
{
	add_to_hl(state, REG_SP(state));

	state->wait = 8;
}

/*!
 * @brief DEC SP (0x3B)
 * @result 1 is subtracted from SP (possibly wrapping)
 */
static inline void dec_sp(emu_state *restrict state, uint8_t data[] UNUSED)
{
	if(state->system < SYSTEM_SGB)
	{
		maybe_corrupt_oam(state, REG_SP(state));
	}
	REG_SP(state)--;

	state->wait = 8;
}

/*!
 * @brief INC A (0x3C)
 * @result 1 is added to A; Z if A is now zero, H if bit 3 overflow
 */
static inline void inc_a(emu_state *restrict state, uint8_t data[] UNUSED)
{
	inc_r8(state, &REG_A(state));
}

/*!
 * @brief DEC A (0x3D)
 * @result 1 is subtracted from A; Z if A is now zero, H if bit 4 underflow
 */
static inline void dec_a(emu_state *restrict state, uint8_t data[] UNUSED)
{
	dec_r8(state, &REG_A(state));
}

static inline void add_common(emu_state *restrict state, uint8_t to_add)
{
	uint32_t temp = REG_A(state) + to_add;

	FLAGS_CLEAR(state);

	if(temp)
	{
		if(temp & 0x100)
		{
			FLAG_SET(state, FLAG_C);
		}

		// Half carry
		if(((REG_A(state) & 0xF) + (to_add & 0xF)) & 0x10)
		{
			FLAG_SET(state, FLAG_H);
		}
	}

	REG_A(state) = (uint8_t)temp;
	if(REG_A(state) == 0)
	{
		FLAG_SET(state, FLAG_Z);
	}

	state->wait = 4;
}

/*!
 * @brief ADD B (0x80)
 * @result A += B
 */
static inline void add_b(emu_state *restrict state, uint8_t data[] UNUSED)
{
	add_common(state, REG_B(state));
}

/*!
 * @brief ADD C (0x81)
 * @result A += C
 */
static inline void add_c(emu_state *restrict state, uint8_t data[] UNUSED)
{
	add_common(state, REG_C(state));
}

/*!
 * @brief ADD D (0x82)
 * @result A += D
 */
static inline void add_d(emu_state *restrict state, uint8_t data[] UNUSED)
{
	add_common(state, REG_D(state));
}

/*!
 * @brief ADD E (0x83)
 * @result A += E
 */
static inline void add_e(emu_state *restrict state, uint8_t data[] UNUSED)
{
	add_common(state, REG_E(state));
}

/*!
 * @brief ADD H (0x84)
 * @result A += H
 */
static inline void add_h(emu_state *restrict state, uint8_t data[] UNUSED)
{
	add_common(state, REG_H(state));
}

/*!
 * @brief ADD L (0x85)
 * @result A += L
 */
static inline void add_l(emu_state *restrict state, uint8_t data[] UNUSED)
{
	add_common(state, REG_L(state));
}

/*!
 * @brief ADD (HL) (0x86)
 * @result A += contents of memory at HL
 */
static inline void add_hl(emu_state *restrict state, uint8_t data[] UNUSED)
{
	add_common(state, mem_read8(state, REG_HL(state)));

	// add_common already adds 4
	state->wait = 4;
}

/*!
 * @brief ADD A (0x87)
 * @result A += A
 */
static inline void add_a(emu_state *restrict state, uint8_t data[] UNUSED)
{
	add_common(state, REG_A(state));
}

static inline void adc_common(emu_state *restrict state, uint8_t to_add)
{
	uint32_t carry = (IS_FLAG(state, FLAG_C)) ? 1 : 0;
	uint32_t temp = REG_A(state) + to_add + carry;

	FLAGS_CLEAR(state);

	if(temp)
	{
		if(temp & 0x100)
		{
			FLAG_SET(state, FLAG_C);
		}

		// Half carry
		if(((REG_A(state) & 0xF) + (to_add & 0xF) + carry) & 0x10)
		{
			FLAG_SET(state, FLAG_H);
		}
	}

	REG_A(state) = (uint8_t)temp;
	if(REG_A(state) == 0)
	{
		FLAG_SET(state, FLAG_Z);
	}

	state->wait = 4;
}

/*!
 * @brief ADC B (0x88)
 * @result A += B (+1 if C flag set)
 */
static inline void adc_b(emu_state *restrict state, uint8_t data[] UNUSED)
{
	adc_common(state, REG_B(state));
}

/*!
 * @brief ADC C (0x89)
 * @result A += C (+1 if C flag set)
 */
static inline void adc_c(emu_state *restrict state, uint8_t data[] UNUSED)
{
	adc_common(state, REG_C(state));
}

/*!
 * @brief ADC D (0x8A)
 * @result A += D (+1 if C flag set)
 */
static inline void adc_d(emu_state *restrict state, uint8_t data[] UNUSED)
{
	adc_common(state, REG_D(state));
}

/*!
 * @brief ADC E (0x8B)
 * @result A += E (+1 if C flag set)
 */
static inline void adc_e(emu_state *restrict state, uint8_t data[] UNUSED)
{
	adc_common(state, REG_E(state));
}

/*!
 * @brief ADC H (0x8C)
 * @result A += H (+1 if C flag set)
 */
static inline void adc_h(emu_state *restrict state, uint8_t data[] UNUSED)
{
	adc_common(state, REG_H(state));
}

/*!
 * @brief ADC L (0x8D)
 * @result A += L (+1 if C flag set)
 */
static inline void adc_l(emu_state *restrict state, uint8_t data[] UNUSED)
{
	adc_common(state, REG_L(state));
}

/*!
 * @brief ADC (HL) (0x8E)
 * @result A += contents of memory at HL (+1 if C flag set)
 */
static inline void adc_hl(emu_state *restrict state, uint8_t data[] UNUSED)
{
	adc_common(state, mem_read8(state, REG_HL(state)));

	// adc_common already adds 4
	state->wait += 4;
}

/*!
 * @brief ADC A (0x8F)
 * @result A += A (+1 if C flag set)
 */
static inline void adc_a(emu_state *restrict state, uint8_t data[] UNUSED)
{
	adc_common(state, REG_A(state));
}

static inline void sub_common(emu_state *restrict state, uint8_t to_sub)
{
	uint32_t temp = REG_A(state) - to_sub;

	FLAGS_OVERWRITE(state, FLAG_N);

	if(REG_A(state) < to_sub)
	{
		FLAG_SET(state, FLAG_C);
	}

	if((REG_A(state) & 0x0f) < (to_sub & 0x0f))
	{
		FLAG_SET(state, FLAG_H);
	}

	REG_A(state) = (uint8_t)temp;
	if(REG_A(state) == 0)
	{
		FLAG_SET(state, FLAG_Z);
	}
}

/*!
 * @brief SUB B (0x90)
 * @result A -= B; Z if A = 0, H if no borrow from bit 4, C if no borrow
 */
static inline void sub_b(emu_state *restrict state, uint8_t data[] UNUSED)
{
	sub_common(state, REG_B(state));
}

/*!
 * @brief SUB C (0x91)
 * @result A -= C; Z if A = 0, H if no borrow from bit 4, C if no borrow
 */
static inline void sub_c(emu_state *restrict state, uint8_t data[] UNUSED)
{
	sub_common(state, REG_C(state));
}

/*!
 * @brief SUB D (0x92)
 * @result A -= D; Z if A = 0, H if no borrow from bit 4, C if no borrow
 */
static inline void sub_d(emu_state *restrict state, uint8_t data[] UNUSED)
{
	sub_common(state, REG_D(state));
}

/*!
 * @brief SUB E (0x93)
 * @result A -= E; Z if A = 0, H if no borrow from bit 4, C if no borrow
 */
static inline void sub_e(emu_state *restrict state, uint8_t data[] UNUSED)
{
	sub_common(state, REG_E(state));
}

/*!
 * @brief SUB H (0x94)
 * @result A -= H; Z if A = 0, H if no borrow from bit 4, C if no borrow
 */
static inline void sub_h(emu_state *restrict state, uint8_t data[] UNUSED)
{
	sub_common(state, REG_H(state));
}

/*!
 * @brief SUB L (0x95)
 * @result A -= L; Z if A = 0, H if no borrow from bit 4, C if no borrow
 */
static inline void sub_l(emu_state *restrict state, uint8_t data[] UNUSED)
{
	sub_common(state, REG_L(state));
}

/*!
 * @brief SUB (HL) (0x96)
 * @result A -= contents of memory at HL
 */
static inline void sub_hl(emu_state *restrict state, uint8_t data[] UNUSED)
{
	sub_common(state, mem_read8(state, REG_HL(state)));

	// sub_common already adds 4
	state->wait += 4;
}

/*!
 * @brief SUB A (0x97)
 * @result A = 0; Z set, H if no borrow from bit 4, C if no borrow
 */
static inline void sub_a(emu_state *restrict state, uint8_t data[] UNUSED)
{
	sub_common(state, REG_A(state));
}

static inline void sbc_common(emu_state *restrict state, uint8_t to_sub)
{
	uint8_t f = IS_FLAG(state, FLAG_C) ? 1 : 0;
	uint8_t temp = REG_A(state) - to_sub - f;

	FLAGS_OVERWRITE(state, FLAG_N);

	if((REG_A(state) - f) < to_sub)
	{
		FLAG_SET(state, FLAG_C);
	}

	if(((REG_A(state) & 0x0f) - f) < (to_sub & 0x0f))
	{
		FLAG_SET(state, FLAG_H);
	}

	REG_A(state) = (uint8_t)temp;
	if(REG_A(state) == 0)
	{
		FLAG_SET(state, FLAG_Z);
	}
}

/*!
 * @brief SBC B (0x98)
 * @result A -= B (+1 if C flag set)
 */
static inline void sbc_b(emu_state *restrict state, uint8_t data[] UNUSED)
{
	sbc_common(state, REG_B(state));
}

/*!
 * @brief SBC C (0x99)
 * @result A -= C (+1 if C flag set)
 */
static inline void sbc_c(emu_state *restrict state, uint8_t data[] UNUSED)
{
	sbc_common(state, REG_C(state));
}

/*!
 * @brief SBC D (0x9A)
 * @result A -= D (+1 if C flag set)
 */
static inline void sbc_d(emu_state *restrict state, uint8_t data[] UNUSED)
{
	sbc_common(state, REG_D(state));
}

/*!
 * @brief SBC E (0x9B)
 * @result A -= E (+1 if C flag set)
 */
static inline void sbc_e(emu_state *restrict state, uint8_t data[] UNUSED)
{
	sbc_common(state, REG_E(state));
}

/*!
 * @brief SBC H (0x9C)
 * @result A -= H (+1 if C flag set)
 */
static inline void sbc_h(emu_state *restrict state, uint8_t data[] UNUSED)
{
	sbc_common(state, REG_H(state));
}

/*!
 * @brief SBC L (0x9D)
 * @result A -= L (+1 if C flag set)
 */
static inline void sbc_l(emu_state *restrict state, uint8_t data[] UNUSED)
{
	sbc_common(state, REG_L(state));
}

/*!
 * @brief SBC (HL) (0x9E)
 * @result A -= contents of memory at HL (+1 if C flag set)
 */
static inline void sbc_hl(emu_state *restrict state, uint8_t data[] UNUSED)
{
	sbc_common(state, mem_read8(state, REG_HL(state)));

	// sbc_common already adds 4
	state->wait += 4;
}

/*!
 * @brief SBC A (0x9F)
 * @result A -= A (+1 if C flag set)
 */
static inline void sbc_a(emu_state *restrict state, uint8_t data[] UNUSED)
{
	sbc_common(state, REG_A(state));
}

/*!
 * @brief ADD n (0xC6)
 * @result A += immediate (n)
 */
static inline void add_d8(emu_state *restrict state, uint8_t data[])
{
	add_common(state, data[0]);

	// add_common already adds 4
	state->wait += 4;
}

/*!
 * @brief ADC n (0xCE)
 * @result A += immediate (n) (+1 if C flag set)
 */
static inline void adc_d8(emu_state *restrict state, uint8_t data[])
{
	adc_common(state, data[0]);

	// adc_common already adds 4
	state->wait += 4;
}

/*!
 * @brief SUB n (0xD6)
 * @result A -= n; Z if A = 0, H if no borrow from bit 4, C if no borrow
 */
static inline void sub_d8(emu_state *restrict state, uint8_t data[])
{
	sub_common(state, data[0]);

	// sub_common already adds 4
	state->wait += 4;
}

/*!
 * @brief SBC n (0xDE)
 * @result A -= n (+1 if C flag set)
 */
static inline void sbc_d8(emu_state *restrict state, uint8_t data[])
{
	sbc_common(state, data[0]);

	// sbc_common already adds 4
	state->wait += 4;
}

/*!
 * @brief ADD SP,n (0xE8)
 * @result SP += n
 */
static inline void add_sp_d8(emu_state *restrict state, uint8_t data[])
{
	int8_t val = (int8_t)data[0];
	uint16_t temp = REG_SP(state) + val;
	uint16_t temp2 = REG_SP(state) ^ val ^ temp;

	REG_SP(state) = temp;

	FLAGS_CLEAR(state);

	if((temp2 & 0x100) == 0x100)
	{
		FLAG_SET(state, FLAG_C);
	}

	if((temp2 & 0x10) == 0x10)
	{
		FLAG_SET(state, FLAG_H);
	}

	state->wait = 16;
}
