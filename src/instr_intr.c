/*!
 * @brief STOP (0x10
 * @result puts the system to sleep until any button is pressed
 * @note also has CGB side effect of changing speed potentially
 */
static inline void stop(emu_state *restrict state, uint8_t data[] unused)
{
	if(state->system == SYSTEM_CGB)
	{
		uint8_t speed_reg = mem_read8(state, 0xFF4D);

		if(speed_reg & 0x1)
		{
			state->freq = ((speed_reg & 0x40) ? CPU_FREQ_DMG : CPU_FREQ_CGB);
		}
		else
		{
			state->stop = true;
		}
	}
	else
	{
		state->stop = true;
	}

	state->wait = 4;
}

/*!
 * @brief DI (0xF3) - disable interrupts
 * @result interrupts will be disabled the instruction AFTER this one
 */
static inline void di(emu_state *restrict state, uint8_t data[] unused)
{
	state->interrupts.next_cycle = INT_NEXT_DISABLE;

	state->wait = 4;
}

/*!
 * @brief EI (0xFB) - enable interrupts
 * @result interrupts will be enabled the instruction AFTER this one
 */
static inline void ei(emu_state *restrict state, uint8_t data[] unused)
{
	state->interrupts.next_cycle = INT_NEXT_ENABLE;

	state->wait = 4;
}


/*!
 * @brief HALT (0x76)
 * @result Put CPU to sleep until next interrupt
 */
static inline void halt(emu_state *restrict state, uint8_t data[] unused)
{
	if(state->interrupts.enabled)
	{
		state->halt = true;
	}

	state->wait = 4;
}

