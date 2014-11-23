/*!
 * @brief POP BC (0xC1)
 * @result BC = memory at SP; SP incremented 2
 */
static inline void pop_bc(emu_state *restrict state)
{
	REG_BC(state) = mem_read16(state, REG_SP(state));
	REG_SP(state) += 2;
	REG_PC(state)++;

	state->wait = 12;
}

/*!
 * @brief PUSH BC (0xC5)
 * @result contents of memory at SP = BC; SP decremented 2
 */
static inline void push_bc(emu_state *restrict state)
{
	REG_SP(state) -= 2;
	mem_write16(state, REG_SP(state), REG_BC(state));
	REG_PC(state)++;

	state->wait = 16;
}

/*!
 * @brief POP DE (0xD1)
 * @result DE = memory at SP; SP incremented 2
 */
static inline void pop_de(emu_state *restrict state)
{
	REG_DE(state) = mem_read16(state, REG_SP(state));
	REG_SP(state) += 2;
	REG_PC(state)++;

	state->wait = 12;
}

/*!
 * @brief PUSH DE (0xD5)
 * @result contents of memory at SP = DE; SP decremented 2
 */
static inline void push_de(emu_state *restrict state)
{
	REG_SP(state) -= 2;
	mem_write16(state, REG_SP(state), REG_DE(state));
	REG_PC(state)++;

	state->wait = 16;
}

/*!
 * @brief POP HL (0xE1)
 * @result HL = memory at SP; SP incremented 2
 */
static inline void pop_hl(emu_state *restrict state)
{
	REG_HL(state) = mem_read16(state, REG_SP(state));
	REG_SP(state) += 2;
	REG_PC(state)++;

	state->wait = 12;
}

/*!
 * @brief PUSH HL (0xE5)
 * @result contents of memory at SP = HL; SP decremented 2
 */
static inline void push_hl(emu_state *restrict state)
{
	REG_SP(state) -= 2;
	mem_write16(state, REG_SP(state), REG_HL(state));
	REG_PC(state)++;

	state->wait = 16;
}

/*!
 * @brief POP AF (0xF1)
 * @result AF = memory at SP; SP incremented 2
 */
static inline void pop_af(emu_state *restrict state)
{
	REG_AF(state) = mem_read16(state, REG_SP(state));
	REG_SP(state) += 2;
	REG_PC(state)++;

	state->wait = 12;
}

/*!
 * @brief PUSH AF (0xF5)
 * @result contents of memory at SP = AF; SP decremented 2
 */
static inline void push_af(emu_state *restrict state)
{
	REG_SP(state) -= 2;
	mem_write16(state, REG_SP(state), REG_AF(state));
	REG_PC(state)++;

	state->wait = 16;
}
