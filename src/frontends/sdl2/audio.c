#include "sgherm.h"	// emu_state,
#include "print.h"	// debug
#include "signals.h"	// do_exit
#include "frontend.h"	// frontend
#include "frontends/sdl2/frontend.h"
#include "frontends/sdl2/sdl_inc.h"	// SDL

static void sdl2_audio_callback(void *userdata, Uint8 *stream, int len)
{
	emu_state *state = (emu_state *)userdata;
	//sdl2_audio_data *ad = (sdl2_audio_data *)(state->front.audio.data);
	sound_fetch_s16ne(state, (int16_t *)stream, (size_t)(len/4));
}

bool sdl2_init_audio(emu_state *state)
{
	info(state, "Initalising the SDL audio frontend");
	//sdl2_audio_data *ad = calloc(1, sizeof(sdl2_audio_data));
	snd_state *snd = &state->snd;
	//state->front.audio.data = (void *)ad;

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

	snd->freq = aspec.freq;

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
