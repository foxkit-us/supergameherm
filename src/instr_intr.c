/*!
 * @brief STOP (0x10
 * @result puts the system to sleep until any button is pressed
 * @note also has CGB side effect of changing speed potentially
 */
static inline void stop(emu_state *restrict state)
{
	// XXX validate for DMG only operation
	uint8_t speed_reg = mem_read8(state, 0xFF4D);

	if(speed_reg & 0x1)
	{
		// XXX simulate flicker
		state->freq = ((speed_reg & 0x40) ? CPU_FREQ_DMG : CPU_FREQ_CGB);
	}
	else
	{
		state->stop = true;
	}

	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief DI (0xF3) - disable interrupts
 * @result interrupts will be disabled the instruction AFTER this one
 */
static inline void di(emu_state *restrict state)
{
	state->interrupts.next_cycle = INT_NEXT_DISABLE;
	REG_PC(state)++;

	state->wait = 4;
}

/*!
 * @brief EI (0xFB) - enable interrupts
 * @result interrupts will be enabled the instruction AFTER this one
 */
static inline void ei(emu_state *restrict state)
{
	state->interrupts.next_cycle = INT_NEXT_ENABLE;
	REG_PC(state)++;

	state->wait = 4;
}


/*!
 * @brief HALT (0x76)
 * @result Put CPU to sleep until next interrupt
 */
static inline void halt(emu_state *restrict state)
{
	if(state->interrupts.enabled)
	{
		state->halt = true;
	}

	REG_PC(state)++;

	state->wait = 4;
}

