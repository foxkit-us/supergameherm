static inline uint8_t joypad_read(emu_state *restrict state, uint16_t reg UNUSED)
{
	return (state->input.col << 4) | key_scan(state);
}

static inline void joypad_write(emu_state *restrict state, uint16_t reg UNUSED, uint8_t data)
{
	state->input.col = data >> 4;
}

