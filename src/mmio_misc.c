static inline uint8_t key1_read(emu_state *restrict state, uint16_t location)
{
	if(state->system != SYSTEM_CGB)
	{
		return no_hw_read(state, location);
	}

	// TODO: get the unused bits
	return 0x3E
	| (state->freq == CPU_FREQ_CGB ? 0x80 : 0x00)
	| (state->key1 ? 0x01 : 0x00)
	;
}

static inline void key1_write(emu_state *restrict state, uint16_t location UNUSED, uint8_t data)
{
	state->key1 = (data & 1 ? true : false);
}

static inline void bootrom_exit(emu_state *restrict state, uint16_t location UNUSED, uint8_t data)
{
	if(!(state->in_bootrom))
	{
		return;
	}

	if(data == 0x01)
	{
		debug(state, "Exiting boot ROM");
		state->in_bootrom = false;
		state->bootrom_size = 0;

		free(state->bootrom_data);
	}
}

static inline void wram_bank_switch_write(emu_state *restrict state, uint16_t location, uint8_t data)
{
	if(state->system != SYSTEM_CGB)
	{
		no_hw_write(state, location, data);
		return;
	}

	data &= 7;
	if(data == 0) data = 1;
	state->wram_bank = data;
}
