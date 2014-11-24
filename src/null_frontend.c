#include "sgherm.h"	// emu_state, unused
#include "print.h"	// debug
#include "frontend.h"	// frontend

bool null_init_video(emu_state *state unused)
{
	static bool did_notice = false;

	if(unlikely(!did_notice))
	{
		debug("Not initialising a null display");
		did_notice = true;
	}

	return true;
}

void null_finish_video(emu_state *state unused)
{
	static bool did_notice = false;

	if(unlikely(!did_notice))
	{
		debug("Not finalising a null display");
		did_notice = true;
	}
}

bool null_init_audio(emu_state *state unused)
{
	static bool did_notice = false;

	if(unlikely(!did_notice))
	{
		debug("Not initialising null audio");
		did_notice = true;
	}

	return true;
}

void null_finish_audio(emu_state *state unused)
{
	static bool did_notice = false;

	if(unlikely(!did_notice))
	{
		debug("Not finalising null audio");
		did_notice = true;
	}
}

bool null_init_input(emu_state *state unused)
{
	static bool did_notice = false;

	if(unlikely(!did_notice))
	{
		debug("Not initialising a null keyboard");
		did_notice = true;
	}

	return true;
}

void null_finish_input(emu_state *state unused)
{
	static bool did_notice = false;

	if(unlikely(!did_notice))
	{
		debug("Not finalising a null keyboard");
		did_notice = true;
	}
}

void null_blit_canvas(emu_state *state unused)
{
	static bool did_notice = false;

	if(unlikely(!did_notice))
	{
		debug("Not blitting to null display");
		did_notice = true;
	}
}

void null_output_sample(emu_state *state unused)
{
	static bool did_notice = false;

	if(unlikely(!did_notice))
	{
		debug("Not outputting to null audio");
		did_notice = true;
	}
}

int null_event_loop(emu_state *state unused)
{
	static bool did_notice = false;

	if(unlikely(!did_notice))
	{
		debug("Executing null event loop");
		did_notice = true;
	}

	return 0;
}


frontend null_frontend = {
	.init_video = &null_init_video,
	.init_audio = &null_init_audio,
	.init_input = &null_init_input,
	.finish_video = &null_finish_video,
	.finish_audio = &null_finish_audio,
	.finish_input = &null_finish_input,
	.blit_canvas = &null_blit_canvas,
	.output_sample = &null_output_sample,
	.event_loop = &null_event_loop,
};
