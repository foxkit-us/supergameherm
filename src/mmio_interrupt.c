static inline uint8_t int_flag_read(emu_state *restrict state, uint16_t location UNUSED)
{
	return state->interrupts.pending;
}

static inline void int_flag_write(emu_state *restrict state, uint16_t location UNUSED, uint8_t data)
{
	state->interrupts.pending = data;
	compute_irq(state);
}

