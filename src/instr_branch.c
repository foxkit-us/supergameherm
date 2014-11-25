/*!
 * @brief JR n (0x18)
 * @result add n to pc
 */
static inline void jr_imm8(emu_state *restrict state)
{
	int8_t to_add = mem_read8(state, ++REG_PC(state));

	REG_PC(state) += to_add + 1;

	state->wait = 12;
}

/*!
 * @brief JR NZ,n (0x20)
 * @result add n to pc if Z (zero) flag clear
 */
static inline void jr_nz_imm8(emu_state *restrict state)
{
	int8_t to_add = mem_read8(state, ++REG_PC(state)) + 1;

	state->wait = 8;

	if(IS_FLAG(state, FLAG_Z))
	{
		REG_PC(state)++;
	}
	else
	{
		REG_PC(state) += to_add;

		state->wait += 4;
	}
}

/*!
 * @brief JR Z,n (0x28)
 * @result add n to pc if Z (zero) flag set
 */
static inline void jr_z_imm8(emu_state *restrict state)
{
	int8_t to_add = mem_read8(state, ++REG_PC(state)) + 1;

	state->wait = 8;

	if(IS_FLAG(state, FLAG_Z))
	{
		REG_PC(state) += to_add;

		state->wait += 4;
	}
	else
	{
		REG_PC(state)++;
	}
}

/*!
 * @brief JR NC,n (0x30)
 * @result add n to pc if C (carry) flag clear
 */
static inline void jr_nc_imm8(emu_state *restrict state)
{
	int8_t to_add = mem_read8(state, ++REG_PC(state)) + 1;

	state->wait = 8;

	if(IS_FLAG(state, FLAG_C))
	{
		REG_PC(state)++;
	}
	else
	{
		REG_PC(state) += to_add;

		state->wait += 4;
	}
}

/*!
 * @brief JR C,n (0x38)
 * @result add n to pc if C (carry) flag set
 */
static inline void jr_c_imm8(emu_state *restrict state)
{
	int8_t to_add = mem_read8(state, ++REG_PC(state)) + 1;

	state->wait = 8;

	if(IS_FLAG(state, FLAG_C))
	{
		REG_PC(state) += to_add;

		state->wait += 4;
	}
	else
	{
		REG_PC(state)++;
	}
}

/*!
 * @brief JP NZ,nn (0xC2)
 * @result pc is set to 16-bit immediate value (LSB, MSB) if Z flag is not set
 */
static inline void jp_nz_imm16(emu_state *restrict state)
{
	state->wait = 12;

	if(IS_FLAG(state, FLAG_Z))
	{
		REG_PC(state) += 3;
	}
	else
	{
		uint8_t lsb = mem_read8(state, ++REG_PC(state));
		uint8_t msb = mem_read8(state, ++REG_PC(state));

		REG_PC(state) = (msb<<8) | lsb;

		state->wait += 4;
	}
}

/*!
 * @brief JP nn (0xC3)
 * @result pc is set to 16-bit immediate value (LSB, MSB)
 */
static inline void jp_imm16(emu_state *restrict state)
{
	REG_PC(state) = mem_read16(state, ++REG_PC(state));

	state->wait = 16;
}

/*!
 * @brief CALL nn (0xCD)
 * @result next pc stored in stack; jump to nn
 */
static inline void call_imm16(emu_state *restrict state)
{
	uint16_t value = mem_read16(state, ++REG_PC(state));
	++REG_PC(state);

	REG_SP(state) -= 2;
	mem_write16(state, REG_SP(state), ++REG_PC(state));

	REG_PC(state) = value;

	state->wait = 24;
}

/*!
 * @brief CALL NZ,nn (0xC4)
 * @result CALL nn if Z flag is not set
 */
static inline void call_nz_imm16(emu_state *restrict state)
{
	if(IS_FLAG(state, FLAG_Z))
	{
		state->wait = 12;
		REG_PC(state) += 3;
	}
	else
	{
		call_imm16(state);
	}
}

static inline void reset_common(emu_state *restrict state)
{
	uint16_t to = mem_read8(state, REG_PC(state)) - 0xC7;

	REG_SP(state) -= 2;
	mem_write16(state, REG_SP(state), ++REG_PC(state));
	REG_PC(state) = to;

	state->wait = 16;
}

/*!
 * @brief RET (0xC9) - return from CALL
 * @result pop two bytes from the stack and jump to that location
 */
static inline void ret(emu_state *restrict state)
{
	REG_PC(state) = mem_read16(state, REG_SP(state));
	REG_SP(state) += 2;

	state->wait = 16;
}

/*!
 * @brief RETNZ (0xC0) - return from CALL if Z flag not set
 * @result RET, if Z flag not set, otherwise nothing.
 */
static inline void retnz(emu_state *restrict state)
{
	if(IS_FLAG(state, FLAG_Z))
	{
		REG_PC(state)++;

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
	if(IS_FLAG(state, FLAG_Z))
	{
		ret(state);

		// ret already adds 16
		state->wait += 4;
	}
	else
	{
		REG_PC(state)++;

		state->wait = 8;
	}
}

/*!
 * @brief JP Z,nn (0xCA)
 * @result pc is set to 16-bit immediate value (LSB, MSB) if Z flag is set
 */
static inline void jp_z_imm16(emu_state *restrict state)
{
	state->wait = 12;

	if(IS_FLAG(state, FLAG_Z))
	{
		REG_PC(state) = mem_read16(state, ++REG_PC(state));

		state->wait += 4;
	}
	else
	{
		REG_PC(state) += 3;
	}
}

/*!
 * @brief CALL Z,nn (0xCC)
 * @result CALL nn if Z flag is set
 */
static inline void call_z_imm16(emu_state *restrict state)
{
	if(IS_FLAG(state, FLAG_Z))
	{
		call_imm16(state);
	}
	else
	{
		state->wait = 12;
		REG_PC(state) += 3;
	}
}

/*!
 * @brief RETNC (0xD0) - return from CALL if C flag not set
 * @result RET, if C flag not set, otherwise nothing.
 */
static inline void retnc(emu_state *restrict state)
{
	if(IS_FLAG(state, FLAG_C))
	{
		REG_PC(state)++;

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
	state->wait = 12;

	if(IS_FLAG(state, FLAG_C))
	{
		REG_PC(state) += 3;
	}
	else
	{
		REG_PC(state) = mem_read16(state, ++REG_PC(state));

		state->wait += 4;
	}
}

/*!
 * @brief CALL NC,nn (0xD4)
 * @result CALL nn if C flag is not set
 */
static inline void call_nc_imm16(emu_state *restrict state)
{
	if(IS_FLAG(state, FLAG_C))
	{
		state->wait = 12;
		REG_PC(state) += 3;
	}
	else
	{
		call_imm16(state);
	}
}

/*!
 * @brief RETC (0xD8) - return from CALL if C flag set
 * @result RET, if C flag is set, otherwise nothing.
 */
static inline void retc(emu_state *restrict state)
{
	if(IS_FLAG(state, FLAG_C))
	{
		ret(state);

		// ret already adds 16
		state->wait += 4;
	}
	else
	{
		REG_PC(state)++;

		state->wait = 8;
	}
}

/*!
 * @brief RETI (0xD9) - return from CALL and enable interrupts
 * @result RET + EI
 */
static inline void reti(emu_state *restrict state)
{
	state->int_state.enabled = true;
	ret(state);
}

/*!
 * @brief JP C,nn (0xDA)
 * @result pc is set to 16-bit immediate value (LSB, MSB) if C flag is set
 */
static inline void jp_c_imm16(emu_state *restrict state)
{
	state->wait = 12;

	if(IS_FLAG(state, FLAG_C))
	{
		REG_PC(state) = mem_read16(state, ++REG_PC(state));

		state->wait += 4;
	}
	else
	{
		REG_PC(state) += 3;
	}
}

/*!
 * @brief CALL C,nn (0xDC)
 * @result CALL nn if C flag is set
 */
static inline void call_c_imm16(emu_state *restrict state)
{
	if(IS_FLAG(state, FLAG_C))
	{
		call_imm16(state);
	}
	else
	{
		state->wait = 12;
		REG_PC(state) += 3;
	}
}

/*!
 * @brief JP HL (0xE9)
 * @result pc = HL
 */
static inline void jp_hl(emu_state *restrict state)
{
	REG_PC(state) = REG_HL(state);

	state->wait = 4;
}
