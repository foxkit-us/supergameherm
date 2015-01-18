static inline uint8_t sound_read(emu_state *restrict state, uint16_t reg)
{
	switch(reg)
	{
	//! NR 10 - ch 1 - sweep
	case 0xFF10:
	{
		uint8_t val = (state->snd.ch1.sweep_time & 0x7) << 4;
		val |= (state->snd.ch1.sweep_dec) << 3;
		val |= (state->snd.ch1.sweep_shift & 0x7);
		return val;
	}
	//! NR 11 - ch 1 - wave pattern duty
	case 0xFF11:
	{
		return state->snd.ch1.wave_duty << 6;
	}
	//! NR 12 - ch 1 - envelope
	case 0xFF12:
	{
		uint8_t val = (state->snd.ch1.envelope_volume << 4);
		val |= (state->snd.ch1.envelope_amp) << 3;
		val |= (state->snd.ch1.envelope_speed & 0x7);
		return val;
	}
	//! NR 13 - ch 1 - period LSB
	case 0xFF13:
	{
		error(state, "sound: read from write-only register FF13");
		return 0xFF;
	}
	//! NR 14 - ch 1 - misc
	case 0xFF14:
	{
		return state->snd.ch1.counter << 6;
	}

	//! NR 21 - ch 2 - wave pattern duty
	case 0xFF16:
	{
		return state->snd.ch2.wave_duty << 6;
	}
	//! NR 22 - ch 2 - envelope
	case 0xFF17:
	{
		uint8_t val = (state->snd.ch2.envelope_volume << 4);
		val |= (state->snd.ch2.envelope_amp) << 3;
		val |= (state->snd.ch2.envelope_speed & 0x7);
		return val;
	}
	//! NR 23 - ch 2 - period LSB
	case 0xFF18:
	{
		error(state, "sound: read from write-only register FF18");
		return 0xFF;
	}
	//! NR 24 - ch 2 - misc
	case 0xFF19:
	{
		return state->snd.ch2.counter << 6;
	}
	//! NR 30 - ch 3 - enable
	case 0xFF1A:
	{
		return state->snd.ch3.enabled << 7;
	}
	//! NR 50 - ch control
	case 0xFF24:
	{
		uint8_t val = (state->snd.s02 << 7) | (state->snd.s01 << 3);
		val |= (state->snd.s02_volume & 0x7) << 4;
		val |= (state->snd.s01_volume & 0x7);
		return val;
	}
	//! NR 51 - because we can say stereo if we want to
	case 0xFF25:
	{
		return  state->snd.ch1.s01 |
			(state->snd.ch2.s01 << 1) |
			(state->snd.ch3.s01 << 2) |
			(state->snd.ch4.s01 << 3) |
			(state->snd.ch1.s02 << 4) |
			(state->snd.ch2.s02 << 5) |
			(state->snd.ch3.s02 << 6) |
			(state->snd.ch4.s02 << 7);
	}
	//! NR 52 - ch status
	case 0xFF26:
	{
		return  (state->snd.enabled << 7) |
			(state->snd.ch4.enabled << 3) |
			(state->snd.ch3.enabled << 2) |
			(state->snd.ch2.enabled << 1) |
			(state->snd.ch1.enabled);
	}
	default:
	{
		//error(state, "sound: unrecognised register %04X (R)", reg);
		return 0xFF;
	}
	}
}

static inline void sound_write(emu_state *restrict state, uint16_t reg, uint8_t data)
{
	if(reg >= 0xFF30 && reg <= 0xFF3F)
	{
		state->snd.ch3.wave[reg - 0xFF30] = data;
		return;
	}

	switch(reg)
	{
	//! NR 10 - ch 1 - sweep
	case 0xFF10:
	{
		state->snd.ch1.sweep_time = (data >> 4) & 0x7;
		state->snd.ch1.sweep_dec = ((data & 0x08) != 0);
		state->snd.ch1.sweep_shift = (data & 0x7);
		break;
	}
	//! NR 11 - ch 1 - length/duty
	case 0xFF11:
	{
		state->snd.ch1.length = (data & 0x3F);
		state->snd.ch1.wave_duty = (data >> 6);
		break;
	}
	//! NR 12 - ch 1 - envelope
	case 0xFF12:
	{
		state->snd.ch1.envelope_volume = (data >> 4);
		state->snd.ch1.envelope_amp = ((data & 0x08) != 0);
		state->snd.ch1.envelope_speed = (data & 0x7);
		break;
	}
	//! NR 13 - ch 1 - period LSB
	case 0xFF13:
	{
		uint16_t val = state->snd.ch1.period & 0xFF00;
		val |= data;
		state->snd.ch1.period = val;
		break;
	}
	//! NR 14 - ch 1 - misc
	case 0xFF14:
	{
		uint16_t val = state->snd.ch1.period & 0x00FF;
		val |= (data & 0x7) << 8;
		state->snd.ch1.period = val;
		state->snd.ch1.initial = ((data & 0x80) != 0);
		state->snd.ch1.counter = ((data & 0x40) != 0);
		break;
	}

	//! NR 21 - ch 2 - length/duty
	case 0xFF16:
	{
		state->snd.ch2.length = (data & 0x3F);
		state->snd.ch2.wave_duty = (data >> 6);
		break;
	}
	//! NR 22 - ch 2 - envelope
	case 0xFF17:
	{
		state->snd.ch2.envelope_volume = (data >> 4);
		state->snd.ch2.envelope_amp = ((data & 0x08) != 0);
		state->snd.ch2.envelope_speed = (data & 0x7);
		break;
	}
	//! NR 23 - ch 2 - period LSB
	case 0xFF18:
	{
		uint16_t val = state->snd.ch2.period & 0xFF00;
		val |= data;
		state->snd.ch2.period = val;
		break;
	}
	//! NR 24 - ch 2 - misc
	case 0xFF19:
	{
		uint16_t val = state->snd.ch2.period & 0x00FF;
		val |= (data & 0x7) << 8;
		state->snd.ch2.period = val;
		state->snd.ch2.initial = ((data & 0x80) != 0);
		state->snd.ch2.counter = ((data & 0x40) != 0);
		break;
	}

	//! NR 30 - ch 3 - enable
	case 0xFF1A:
	{
		state->snd.ch3.enabled = (data & 0x80) != 0;

		if(!state->snd.ch3.enabled)
		{
			state->snd.ch3.per_remain = 0;
			state->snd.ch3.initial = false;
			state->snd.ch3.outseq = 31;
		}

		break;
	}
	//! NR 31 - ch 3 - length
	case 0xFF1B:
	{
		state->snd.ch3.length = data;
		break;
	}
	//! NR 32 - ch 3 - volume
	case 0xFF1C:
	{
		state->snd.ch3.volume = ((data >> 5) & 0x03);
		break;
	}
	//! NR 33 - ch 3 - period LSB
	case 0xFF1D:
	{
		uint16_t val = state->snd.ch3.period & 0xFF00;
		val |= data;
		state->snd.ch3.period = val;
		break;
	}
	//! NR 34 - ch 3 - misc
	case 0xFF1E:
	{
		uint16_t val = state->snd.ch3.period & 0x00FF;
		val |= (data & 0x7) << 8;
		state->snd.ch3.period = val;
		state->snd.ch3.initial = ((data & 0x80) != 0);
		state->snd.ch3.counter = ((data & 0x40) != 0);
		break;
	}

	//! NR 41 - ch 4 - length
	case 0xFF20:
	{
		state->snd.ch4.length = (data & 0x3F);
		break;
	}
	//! NR 42 - ch 4 - envelope
	case 0xFF21:
	{
		state->snd.ch4.envelope_volume = (data >> 4);
		state->snd.ch4.envelope_amp = ((data & 0x08) != 0);
		state->snd.ch4.envelope_speed = (data & 0x7);
		break;
	}
	//! NR 43 - ch 4 - period/LFSR
	case 0xFF22:
	{
		state->snd.ch4.period_exp = (data >> 4);
		state->snd.ch4.is_short = ((data & 0x08) != 0);
		state->snd.ch4.period_mul = (data & 0x07);
		break;
	}
	//! NR 44 - ch 4 - misc
	case 0xFF23:
	{
		state->snd.ch4.initial = ((data & 0x80) != 0);
		state->snd.ch4.counter = ((data & 0x40) != 0);
		break;
	}

	//! NR 50 - ch control
	case 0xFF24:
	{
		state->snd.s02 = ((data & 0x80) != 0);
		state->snd.s01 = ((data & 0x08) != 0);
		state->snd.s02_volume = ((data & 0x70) >> 4);
		state->snd.s01_volume = (data & 0x07);
		break;
	}
	//! NR 51 - 'stereo' emulation
	case 0xFF25:
	{
		state->snd.ch1.s01 = ((data & 0x01) != 0);
		state->snd.ch2.s01 = ((data & 0x02) != 0);
		state->snd.ch3.s01 = ((data & 0x04) != 0);
		state->snd.ch4.s01 = ((data & 0x08) != 0);
		state->snd.ch1.s02 = ((data & 0x10) != 0);
		state->snd.ch2.s02 = ((data & 0x20) != 0);
		state->snd.ch3.s02 = ((data & 0x40) != 0);
		state->snd.ch4.s02 = ((data & 0x80) != 0);
		break;
	}
	//! NR 52 - sound enable
	case 0xFF26:
	{
		state->snd.enabled = ((data & 0x80) != 0);
		break;
	}
	default:
		//error(state, "sound: unrecognised register %04X (W)", reg);
		break;
	}
}

