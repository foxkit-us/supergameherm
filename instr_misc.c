/*!
 * @brief Unimplemented opcode (multiple values)
 * @result Terminates emulator
 * @note This function will go away when all opcodes are implemented
 */
static inline void not_impl(emu_state *restrict state)
{
	uint8_t opcode = mem_read8(state, state->registers.pc);
	fatal("Unimplemented opcode %2X at %4X (mnemonic %s)", opcode,
			state->registers.pc, lookup_mnemonic(opcode));
}

/*!
 * @brief Invalid opcode (multiple values)
 * @result Terminates emulator
 */
static inline void invalid(emu_state *restrict state)
{
	uint8_t opcode = mem_read8(state, state->registers.pc);
	fatal("Invalid opcode %2X at %4X", opcode, state->registers.pc);
}

/*!
 * @brief NOP (0x00)
 * @result Nothing.
 */
static inline void nop(emu_state *restrict state)
{
	state->registers.pc++;

	state->wait = 4;
}
