#include "config.h"	// Various macros, uint[XX]_t

#include "print.h"
#include "sgherm.h"	// emu_state

uint8_t sound_read(emu_state *restrict state, uint16_t reg)
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

void sound_write(emu_state *restrict state, uint16_t reg, uint8_t data)
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
		state->snd.ch1.sweep_dec = ((data & 0x08) == 0x08);
		state->snd.ch1.sweep_shift = (data & 0x7);
		break;
	}
	//! NR 11 - ch 1 - legnth/duty
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
		state->snd.ch1.envelope_amp = ((data & 0x08) == 0x08);
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
		state->snd.ch1.counter = ((data & 0x40) == 0x40);
		break;
	}
	//! NR 21 - ch 2 - legnth/duty
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
		state->snd.ch2.envelope_amp = ((data & 0x08) == 0x08);
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
		state->snd.ch2.counter = ((data & 0x40) == 0x40);
		break;
	}
	//! NR 50 - ch control
	case 0xFF24:
	{
		state->snd.s02 = ((data & 0x80) == 0x80);
		state->snd.s01 = ((data & 0x08) == 0x08);
		state->snd.s02_volume = ((data & 0x70) >> 4);
		state->snd.s01_volume = (data & 0x07);
		break;
	}
	//! NR 51 - 'stereo' emulation
	case 0xFF25:
	{
		state->snd.ch1.s01 = ((data & 0x01) == 0x01);
		state->snd.ch2.s01 = ((data & 0x02) == 0x02);
		state->snd.ch3.s01 = ((data & 0x04) == 0x04);
		state->snd.ch4.s01 = ((data & 0x08) == 0x08);
		state->snd.ch1.s02 = ((data & 0x10) == 0x10);
		state->snd.ch2.s02 = ((data & 0x20) == 0x20);
		state->snd.ch3.s02 = ((data & 0x40) == 0x40);
		state->snd.ch4.s02 = ((data & 0x80) == 0x80);
		break;
	}
	//! NR 52 - sound enable
	case 0xFF26:
	{
		state->snd.enabled = ((data & 0x80) == 0x80);
		break;
	}
	default:
		//error(state, "sound: unrecognised register %04X (W)", reg);
		break;
	}
}

const uint8_t au_pulses[4] = { 0x80, 0xC0, 0xF0, 0x3F, };

void sound_fetch_s16ne(emu_state *restrict state, int16_t *restrict outbuf, size_t len_samples)
{
	size_t i;

	snd_state *snd = &state->snd;

	for(i = 0; i < len_samples; i++)
	{
		// Initialise
		int16_t vl = 0;
		int16_t vr = 0;

		// Calculate period increment
		// TODO: drop the divide, use shift instead - ARMv6 does div in software!
		int32_t pinc = ((1<<(22-2)) + snd->freq_rem) / snd->freq;
		snd->freq_rem = ((1<<(22-2)) + snd->freq_rem) % snd->freq;

		// Update envelopes
		snd->per_env += pinc;
		while(snd->per_env >= (1<<(22-6)))
		{
			snd->per_env -= (1<<(22-6));

			// CH1 env
			if(snd->ch1.envelope_amp) snd->ch1.envelope_volume += snd->ch1.envelope_speed;
			else snd->ch1.envelope_volume -= snd->ch1.envelope_speed;
			if(snd->ch1.envelope_volume < 0) snd->ch1.envelope_volume = 0;
			else if(snd->ch1.envelope_volume > 15) snd->ch1.envelope_volume = 15;

			// CH2 env
			if(snd->ch2.envelope_amp) snd->ch2.envelope_volume += snd->ch2.envelope_speed;
			else snd->ch2.envelope_volume -= snd->ch2.envelope_speed;
			if(snd->ch2.envelope_volume < 0) snd->ch2.envelope_volume = 0;
			else if(snd->ch2.envelope_volume > 15) snd->ch2.envelope_volume = 15;
		}

		// TODO: shift some of this stuff out
		// TODO: once we've done that, make this more accurate
		// Update period
		if(1 || snd->ch1.enabled) snd->ch1.per_remain += pinc;
		if(1 || snd->ch2.enabled) snd->ch2.per_remain += pinc;

		if(snd->ch1.period > 0)
		while(snd->ch1.per_remain >= 0x0800)
		{
			snd->ch1.per_remain += snd->ch1.period - 0x0800;
			snd->ch1.outseq += 1;
			snd->ch1.outseq &= 7;
		}

		if(snd->ch2.period > 0)
		while(snd->ch2.per_remain >= 0x0800)
		{
			snd->ch2.per_remain += snd->ch2.period - 0x0800;
			snd->ch2.outseq += 1;
			snd->ch2.outseq &= 7;
		}

		// TODO: other channels

		if((au_pulses[snd->ch1.wave_duty] >> snd->ch1.outseq) & 1)
		{
			if(snd->ch1.s01) vl += snd->ch1.envelope_volume;
			if(snd->ch1.s02) vr += snd->ch1.envelope_volume;
		}

		if((au_pulses[snd->ch2.wave_duty] >> snd->ch2.outseq) & 1)
		{
			if(snd->ch2.s01) vl += snd->ch2.envelope_volume;
			if(snd->ch2.s02) vr += snd->ch2.envelope_volume;
		}

		// Amplify
		vl <<= 6;
		vr <<= 6;
		vl *= (snd->s01 ? 0 : snd->s01_volume);
		vr *= (snd->s02 ? 0 : snd->s02_volume);

		// Write
		*(outbuf++) = vl;
		*(outbuf++) = vr;
	}
}

void sound_tick(emu_state *restrict state)
{
	// no point if we're disabled.
	if(!state->snd.enabled)
	{
		return;
	}
}
