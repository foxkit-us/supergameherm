/*!
 * @brief Invalid opcode (multiple values)
 * @result Terminates emulator
 */
static inline void invalid(emu_state *restrict state, uint8_t data[] UNUSED)
{
	uint8_t opcode = mem_read8(state, REG_PC(state));
	fatal("Invalid opcode %2X at %4X", opcode, REG_PC(state));
}

/*!
 * @brief NOP (0x00)
 * @result Nothing.
 */
static inline void nop(emu_state *restrict state, uint8_t data[] UNUSED)
{
	state->wait = 4;
}
