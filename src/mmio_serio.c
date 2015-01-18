/*!
 * @brief	Read a register from the serial controller.
 * @param	state	The emulator state to use while reading.
 * @param	reg	The register to read from.
 * @returns	The contents of the selected register.
 */
static inline uint8_t serial_read(emu_state *restrict state, uint16_t reg)
{
	switch(reg)
	{
	case 0xFF01:	// SB - data to read
		return state->ser.out;
	case 0xFF02:	// SC - serial control
	{
		uint8_t res = 0;
		if(state->ser.enabled)
		{
			res |= 0x80;
		}
		if(state->ser.use_internal)
		{
			res |= 0x01;
		}
		return res;
	}
	default:
		error(state, "serial: unknown register %04X (R)", reg);
		return 0xFF;
	}
}

/*!
 * @brief	Write to a register on the serial controller.
 * @param	state	The emulator state to use while writing.
 * @param	reg	The register to write to.
 * @param	data	The data to write to the selected register.
 * @result	The serial controller state is updated.
 */
static inline void serial_write(emu_state *restrict state, uint16_t reg, uint8_t data)
{
	switch(reg)
	{
	case 0xFF01:	// SB - data to write
		//fprintf(to_stdout, "%c", data);
		state->ser.cur_bit = 7;
		state->ser.out = data;
		break;
	case 0xFF02:	// SC - serial control
		state->ser.enabled = (data & 0x80) == 0x80;
		state->ser.use_internal = (data & 0x01) == 0x01;
		break;
	default:
		error(state, "serial: unknown register %04X (W)", reg);
		break;
	}
}

