#include "sgherm.h"	// emu_state, UNUSED
#include "print.h"	// debug
#include "signals.h"	// do_exit
#include "input.h"	// int
#include "frontend.h"	// frontend

bool null_init_video(emu_state *state)
{
	static bool did_notice = false;

	if(unlikely(!did_notice))
	{
		debug(state, "Not initialising a null display");
		did_notice = true;
	}

	return true;
}

void null_finish_video(emu_state *state)
{
	static bool did_notice = false;

	if(unlikely(!did_notice))
	{
		debug(state, "Not finalising a null display");
		did_notice = true;
	}
}

bool null_init_audio(emu_state *state)
{
	static bool did_notice = false;

	if(unlikely(!did_notice))
	{
		debug(state, "Not initialising null audio");
		did_notice = true;
	}

	return true;
}

void null_finish_audio(emu_state *state)
{
	static bool did_notice = false;

	if(unlikely(!did_notice))
	{
		debug(state, "Not finalising null audio");
		did_notice = true;
	}
}

bool null_init_input(emu_state *state)
{
	static bool did_notice = false;

	if(unlikely(!did_notice))
	{
		debug(state, "Not initialising a null keyboard");
		did_notice = true;
	}

	return true;
}

void null_finish_input(emu_state *state)
{
	static bool did_notice = false;

	if(unlikely(!did_notice))
	{
		debug(state, "Not finalising a null keyboard");
		did_notice = true;
	}
}

void null_blit_canvas(emu_state *state)
{
	static bool did_notice = false;

	if(unlikely(!did_notice))
	{
		debug(state, "Not blitting to null display");
		did_notice = true;
	}
}

void null_output_sample(emu_state *state)
{
	static bool did_notice = false;

	if(unlikely(!did_notice))
	{
		debug(state, "Not outputting to null audio");
		did_notice = true;
	}
}

void null_get_key(emu_state *state, frontend_input_return *ret UNUSED)
{
	static bool did_notice = false;

	if(unlikely(!did_notice))
	{
		debug(state, "Not getting a null keystroke");
		did_notice = true;
	}
}

int null_event_loop(emu_state *state)
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

