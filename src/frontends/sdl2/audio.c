#include "sgherm.h"	// emu_state,
#include "print.h"	// debug
#include "signals.h"	// do_exit
#include "frontend.h"	// frontend
#include "frontends/sdl2/sdl_inc.h"	// SDL

uint8_t au_pulses[4] = { 0x80, 0xC0, 0xF0, 0x3F, };

static void sdl2_audio_callback(void *userdata, Uint8 *stream, int len)
{
	int i;

	emu_state *state = (emu_state *)userdata;
	sdl2_audio_data *ad = (sdl2_audio_data *)(state->front.audio.data);
	snd_state *snd = &state->snd;

	//printf("update %i\n", len);

	int16_t *outbuf = (int16_t *)stream;

	for(i = 0; i < len/4; i++)
	{
		// Initialise
		int16_t vl = 0;
		int16_t vr = 0;

		// Calculate period increment
		// TODO: drop the divide, use shift instead - ARMv6 does div in software!
		int32_t pinc = ((1<<(22-2)) + ad->freq_rem) / ad->freq;
		ad->freq_rem = ((1<<(22-2)) + ad->freq_rem) % ad->freq;

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

	//printf("Done\n");
}

bool sdl2_init_audio(emu_state *state)
{
	info(state, "Initalising the SDL audio frontend");
	sdl2_audio_data *ad = calloc(1, sizeof(sdl2_audio_data));
	state->front.audio.data = (void *)ad;

	if(SDL_InitSubSystem(SDL_INIT_AUDIO) != 0)
	{
		error(state, "Failed to initalise audio frontend: %s", SDL_GetError());
		return false;
	}

	SDL_AudioSpec aspec;
	aspec.freq = 44100;
	aspec.format = AUDIO_S16SYS;
	aspec.channels = 2;
	aspec.callback = sdl2_audio_callback;
	aspec.userdata = (void *)state;
	aspec.samples = 512;

	if(SDL_OpenAudio(&aspec, NULL) < 0)
	{
		error(state, "Failed to initalise audio frontend: SDL_OpenAudio: %s", SDL_GetError());
		return false;
	}

	ad->freq = aspec.freq;
	
	SDL_PauseAudio(0);

	return true;
}

void sdl2_finish_audio(emu_state *state UNUSED)
{
	info(state, "SDL audio frontend finishing up");

	SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

void sdl2_output_sample(emu_state *state UNUSED)
{
	// TODO
	return;
}


const frontend_audio sdl2_frontend_audio =
{
	&sdl2_init_audio,
	&sdl2_finish_audio,
	&sdl2_output_sample,
	NULL,
};
