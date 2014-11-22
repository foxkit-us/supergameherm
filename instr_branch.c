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
 * @brief JP HL (0xE9)
 * @result pc = HL
 */
static inline void jp_hl(emu_state *restrict state)
{
	state->registers.pc = state->registers.hl;

	state->wait = 4;
}
