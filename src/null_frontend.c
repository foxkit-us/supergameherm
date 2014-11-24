#include "sgherm.h"	// emu_state, unused
#include "print.h"	// debug
#include "frontend.h"	// frontend

bool null_init_video(emu_state *state unused)
{
	debug("Not initialising a null display");

	return true;
}

void null_finish_video(emu_state *state unused)
{
	debug("Not finalising a null display");
}

bool null_init_audio(emu_state *state unused)
{
	debug("Not initialising null audio");

	return true;
}

void null_finish_audio(emu_state *state unused)
{
	debug("Not finalising null audio");
}

bool null_init_input(emu_state *state unused)
{
	debug("Not initialising a null keyboard");

	return true;
}

void null_finish_input(emu_state *state unused)
{
	debug("Not finalising a null keyboard");
}

void null_blit_canvas(emu_state *state unused)
{
	debug("Not blitting to null display");
}

void null_output_sample(emu_state *state unused)
{
	debug("Not outputting to null audio");
}

int null_event_loop(emu_state *state unused)
{
	debug("Executing null event loop");

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
