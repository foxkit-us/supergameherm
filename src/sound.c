#include "config.h"	// Various macros, uint[XX]_t

#include "print.h"
#include "sgherm.h"	// emu_state

const uint8_t au_pulses[4] = { 0x80, 0xC0, 0xF0, 0x3F, };

void sound_fetch_s16ne(emu_state *restrict state, int16_t *restrict outbuf, size_t len_samples)
{
	size_t i;
	uint16_t lfsr_tap;

	snd_state *snd = &state->snd;

	int ch1_period = snd->ch1.period - 0x0800; 
	int ch2_period = snd->ch2.period - 0x0800; 
	int ch3_period = snd->ch3.period - 0x0800; 
	int ch4_period = ((snd->ch4.period_mul == 0
			? 1
			: (snd->ch4.period_mul<<1))
		<<(snd->ch4.period_exp+0));

	if(snd->ch4.lfsr == 0) snd->ch4.lfsr = 0xFFFF;
	lfsr_tap = (snd->ch4.is_short
		? 0x4040
		: 0x4000);
	//printf("per %c %i\n", snd->ch4.is_short?'s':'L', ch4_period);

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

			// CH3 has no env

			// CH4 env
			if(snd->ch4.envelope_amp) snd->ch4.envelope_volume += snd->ch4.envelope_speed;
			else snd->ch4.envelope_volume -= snd->ch4.envelope_speed;
			if(snd->ch4.envelope_volume < 0) snd->ch4.envelope_volume = 0;
			else if(snd->ch4.envelope_volume > 15) snd->ch4.envelope_volume = 15;
		}

		// TODO: shift some of this stuff out
		// TODO: once we've done that, make this more accurate
		// Update period
		if(snd->ch1.initial) snd->ch1.per_remain += pinc;
		if(snd->ch2.initial) snd->ch2.per_remain += pinc;
		if(snd->ch3.enabled && snd->ch3.initial) snd->ch3.per_remain += pinc;
		if(snd->ch4.initial) snd->ch4.per_remain -= pinc;

		if(snd->ch1.period > 0)
		while(snd->ch1.per_remain >= 0x0800)
		{
			snd->ch1.per_remain += ch1_period;
			snd->ch1.outseq += 1;
			snd->ch1.outseq &= 7;
		}

		if(snd->ch2.period > 0)
		while(snd->ch2.per_remain >= 0x0800)
		{
			snd->ch2.per_remain += ch2_period;
			snd->ch2.outseq += 1;
			snd->ch2.outseq &= 7;
		}

		if(snd->ch3.period > 0)
		while(snd->ch3.per_remain >= 0x0800)
		{
			snd->ch3.per_remain += ch3_period;
			snd->ch3.outseq += 1;
			snd->ch3.outseq &= 31;
		}

		if(ch4_period > 0)
		while(snd->ch4.per_remain < 0)
		{
			// Update LFSR
			int lbit = (snd->ch4.lfsr^(snd->ch4.lfsr>>1))&1;

			snd->ch4.per_remain += ch4_period;

			snd->ch4.lfsr >>= 1;

			if(lbit != 0)
			{
				snd->ch4.lfsr ^= lfsr_tap;
			}
		}

		// CH 1
		if(snd->ch1.initial)
		if((au_pulses[snd->ch1.wave_duty] >> snd->ch1.outseq) & 1)
		{
			if(snd->ch1.s01) vl += snd->ch1.envelope_volume;
			if(snd->ch1.s02) vr += snd->ch1.envelope_volume;
		}

		// CH 2
		if(snd->ch2.initial)
		if((au_pulses[snd->ch2.wave_duty] >> snd->ch2.outseq) & 1)
		{
			if(snd->ch2.s01) vl += snd->ch2.envelope_volume;
			if(snd->ch2.s02) vr += snd->ch2.envelope_volume;
		}

		// CH 3
		if(snd->ch3.enabled && snd->ch3.initial && snd->ch3.volume != 0)
		{
			uint8_t v3 = snd->ch3.wave[snd->ch3.outseq>>1];
			v3 = ((snd->ch3.outseq&1) != 0
				? v3>>4
				: v3&0x0F);

			if(snd->ch3.s01) vl += v3 >> (snd->ch3.volume-1);
			if(snd->ch3.s01) vr += v3 >> (snd->ch3.volume-1);
		}

		// CH 4
		if(snd->ch4.initial)
		if((~snd->ch4.lfsr) & 1)
		{
			if(snd->ch4.s01) vl += snd->ch4.envelope_volume;
			if(snd->ch4.s02) vr += snd->ch4.envelope_volume;
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

void sound_tick(emu_state *restrict state, int count UNUSED)
{
#ifdef DEFENSIVE
	// no point if we're disabled.
	if(!state->snd.enabled)
	{
		return;
	}
#endif

	// Call output_sample
	// TODO: respect count argument
	OUTPUT_SAMPLE(state);

}
