#include "config.h"	// bool
#include "sgherm.h"	// emu_state, UNUSED
#include "print.h"	// debug
#include "signals.h"	// do_exit
#include "input.h"	// int
#include "frontend.h"	// frontend

#include <string.h>	// memcpy


bool select_frontend_input(emu_state *restrict state, const frontend_input *restrict input)
{
	if(state->front.input_set)
	{
		FRONTEND_FINISH_INPUT(state);
	}

	memcpy(&state->front.input, input, sizeof(frontend_input));

	state->front.input_set = FRONTEND_INIT_INPUT(state);
	return state->front.input_set;
}

bool select_frontend_audio(emu_state *restrict state, const frontend_audio *restrict audio)
{
	if(state->front.audio_set)
	{
		FRONTEND_FINISH_AUDIO(state);
	}

	memcpy(&state->front.audio, audio, sizeof(frontend_audio));

	state->front.audio_set = FRONTEND_INIT_AUDIO(state);
	return state->front.audio_set;
}

bool select_frontend_video(emu_state *restrict state, const frontend_video *restrict video)
{
	if(state->front.video_set)
	{
		FRONTEND_FINISH_VIDEO(state);
	}

	memcpy(&state->front.video, video, sizeof(frontend_video));

	state->front.video_set = FRONTEND_INIT_VIDEO(state);
	return state->front.video_set;
}

bool select_frontend_all(emu_state *restrict state, const frontend_input *restrict input,
		const frontend_audio *restrict audio, const frontend_video *restrict video,
		int (*event_loop)(emu_state *))
{
	if(!select_frontend_video(state, video))
	{
		return false;
	}

	if(!select_frontend_input(state, input))
	{
		FRONTEND_FINISH_VIDEO(state);
		return false;
	}

	if(!select_frontend_audio(state, audio))
	{
		FRONTEND_FINISH_INPUT(state);
		FRONTEND_FINISH_VIDEO(state);
		return false;
	}

	state->front.event_loop = event_loop;

	return true;
}

void finish_frontend(emu_state *restrict state)
{
	if(state->front.audio_set)
	{
		FRONTEND_FINISH_AUDIO(state);
	}

	if(state->front.input_set)
	{
		FRONTEND_FINISH_INPUT(state);
	}

	if(state->front.video_set)
	{
		FRONTEND_FINISH_VIDEO(state);
	}

	state->front.audio_set = state->front.input_set =
		state->front.video_set = false;
}

/* Below are the null implementations of frontend functions
 * They can be used as stubs when a given frontend does not implement
 * something, or we have no available frontends for the given thing.
 */

bool null_init_video(emu_state *restrict state)
{
	static bool did_notice = false;

	if(unlikely(!did_notice))
	{
		debug(state, "Not initialising a null display");
		did_notice = true;
	}

	return true;
}

void null_finish_video(emu_state *restrict state)
{
	static bool did_notice = false;

	if(unlikely(!did_notice))
	{
		debug(state, "Not finalising a null display");
		did_notice = true;
	}
}

bool null_init_audio(emu_state *restrict state)
{
	static bool did_notice = false;

	if(unlikely(!did_notice))
	{
		debug(state, "Not initialising null audio");
		did_notice = true;
	}

	return true;
}

void null_finish_audio(emu_state *restrict state)
{
	static bool did_notice = false;

	if(unlikely(!did_notice))
	{
		debug(state, "Not finalising null audio");
		did_notice = true;
	}
}

bool null_init_input(emu_state *restrict state)
{
	static bool did_notice = false;

	if(unlikely(!did_notice))
	{
		debug(state, "Not initialising a null keyboard");
		did_notice = true;
	}

	return true;
}

void null_finish_input(emu_state *restrict state)
{
	static bool did_notice = false;

	if(unlikely(!did_notice))
	{
		debug(state, "Not finalising a null keyboard");
		did_notice = true;
	}
}

void null_blit_canvas(emu_state *restrict state)
{
	static bool did_notice = false;

	if(unlikely(!did_notice))
	{
		debug(state, "Not blitting to null display");
		did_notice = true;
	}
}

void null_output_sample(emu_state *restrict state)
{
	static bool did_notice = false;

	if(unlikely(!did_notice))
	{
		debug(state, "Not outputting to null audio");
		did_notice = true;
	}
}

void null_get_key(emu_state *restrict state, frontend_input_return *ret UNUSED)
{
	static bool did_notice = false;

	if(unlikely(!did_notice))
	{
		debug(state, "Not getting a null keystroke");
		did_notice = true;
	}
}

int null_event_loop(emu_state *restrict state)
{
	debug(state, "Executing null event loop");

	do
	{
		step_emulator(state);
	} while(!do_exit);

	return 0;
}


const frontend_input null_frontend_input = {
	&null_init_input,
	&null_finish_input,
	&null_get_key,
	NULL,
};

const frontend_audio null_frontend_audio = {
	&null_init_audio,
	&null_finish_audio,
	&null_output_sample,
	NULL,
};

const frontend_video null_frontend_video = {
	&null_init_video,
	&null_finish_video,
	&null_blit_canvas,
	NULL,
};
