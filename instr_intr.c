/*!
 * @brief DI (0xF3) - disable interrupts
 * @result interrupts will be disabled the instruction AFTER this one
 */
static inline void di(emu_state *restrict state)
{
	state->iflags |= I_DISABLE_INT_ON_NEXT;
	state->registers.pc++;

	state->wait = 4;
}

/*!
 * @brief EI (0xFB) - enable interrupts
 * @result interrupts will be enabled the instruction AFTER this one
 */
static inline void ei(emu_state *restrict state)
{
	state->iflags |= I_ENABLE_INT_ON_NEXT;
	state->registers.pc++;

	state->wait = 4;
}
