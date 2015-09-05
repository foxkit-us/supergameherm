/*!
 * @brief STOP (0x10)
 * @result puts the system to sleep until any button is pressed
 * @note also has CGB side effect of changing speed potentially
 */
static inline void stop(emu_state *restrict state, uint8_t data[] UNUSED)
{
	if(state->system == SYSTEM_CGB)
	{
		if(state->key1)
		{
			//state->freq = ((speed_reg & 0x40) ? CPU_FREQ_DMG : CPU_FREQ_CGB);
			state->freq = (state->freq == CPU_FREQ_CGB ? CPU_FREQ_DMG : CPU_FREQ_CGB);
			state->step_core = (state->freq == CPU_FREQ_CGB ? 2 : 1);
			printf("Frequency transition to %i\n", state->freq);
			state->key1 = false;
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
static inline void di(emu_state *restrict state, uint8_t data[] UNUSED)
{
	state->interrupts.enabled = false;
	state->interrupts.irq = 0;

	state->wait = 4;
}

/*!
 * @brief EI (0xFB) - enable interrupts
 * @result interrupts will be enabled the instruction AFTER this one
 */
static inline void ei(emu_state *restrict state, uint8_t data[] UNUSED)
{
	// Next instruction isn't halt, so immediately set flag
	if(mem_read8(state, REG_PC(state)) == 0x76)
	{
		state->interrupts.enable_ctr = 1;
	}
	else
	{
		state->interrupts.enable_ctr = 2;
	}

	state->wait = 4;
}

/*!
 * @brief HALT (0x76)
 * @result Put CPU to sleep until next interrupt
 */
static inline void halt(emu_state *restrict state, uint8_t data[] UNUSED)
{
	state->halt = true;

	state->wait = 4;
}

