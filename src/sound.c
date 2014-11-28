#include "config.h"	// Various macros, uint[XX]_t

#include "print.h"
#include "sgherm.h"	// emu_state

uint8_t sound_read(emu_state *restrict state, uint16_t reg)
{
	switch(reg)
	{
	/*! NR 10 - ch 1 - sweep */
	case 0xFF10:
	{
		uint8_t val = (state->snd_state.ch1.sweep_time & 0x7) << 4;
		val |= (state->snd_state.ch1.sweep_dec) << 3;
		val |= (state->snd_state.ch1.shift & 0x7);
		return val;
	}
	/*! NR 11 - ch 1 - wave pattern duty */
	case 0xFF11:
	{
		return state->snd_state.ch1.wave_duty << 6;
	}
	/*! NR 12 - ch 1 - envelope */
	case 0xFF12:
	{
		uint8_t val = (state->snd_state.ch1.envelope_volume << 4);
		val |= (state->snd_state.ch1.envelope_amp) << 3;
		val |= (state->snd_state.ch1.sweep & 0x7);
		return val;
	}
	/*! NR 13 - ch 1 - frequency LSB */
	case 0xFF13:
	{
		error("sound: read to write-only register FF13");
		return 0xFF;
	}
	/*! NR 14 - ch 1 - misc */
	case 0xFF14:
	{
		return state->snd_state.ch1.counter << 6;
	}
	default:
	{
		error("sound: unrecognised register %04X (R)", reg);
		return 0xFF;
	}
	}
}

void sound_write(emu_state *restrict state, uint16_t reg, uint8_t data)
{
	if(reg >= 0xFF30 && reg <= 0xFF3F)
	{
		state->snd_state.ch3.wave[reg - 0xFF30] = data;
		return;
	}

	switch(reg)
	{
	/*! NR 10 - ch 1 - sweep */
	case 0xFF10:
	{
		state->snd_state.ch1.sweep_time = (data >> 4) & 0x7;
		state->snd_state.ch1.sweep_dec = ((data & 0x08) == 0x08);
		state->snd_state.ch1.shift = (data & 0x7);
		break;
	}
	/*! NR 11 - ch 1 - legnth/duty */
	case 0xFF11:
	{
		state->snd_state.ch1.length = (data & 0x3F);
		state->snd_state.ch1.wave_duty = (data >> 6);
		break;
	}
	/*! NR 12 - ch 1 - envelope */
	case 0xFF12:
	{
		state->snd_state.ch1.envelope_volume = (data >> 4);
		state->snd_state.ch1.envelope_amp = ((data & 0x08) == 0x08);
		state->snd_state.ch1.sweep = (data & 0x7);
		break;
	}
	/*! NR 13 - ch 1 - frequency LSB */
	case 0xFF13:
	{
		uint16_t val = state->snd_state.ch1.frequency & 0xFF00;
		val |= data;
		state->snd_state.ch1.frequency = val;
		break;
	}
	/*! NR 14 - ch 1 - misc */
	case 0xFF14:
	{
		uint16_t val = state->snd_state.ch1.frequency & 0x00FF;
		val |= (data & 0x7) << 8;
		state->snd_state.ch1.frequency = val;
		state->snd_state.ch1.counter = ((data & 0x40) == 0x40);
		break;
	}
	default:
		error("sound: unrecognised register %04X (W)", reg);
	}
}

void sound_tick(emu_state *restrict state)
{
	/* no point if we're disabled. */
	if(!state->snd_state.enabled)
	{
		return;
	}
}
