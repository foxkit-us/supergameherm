#include "config.h"	// bool
#include "frontend.h"	// frontend

/*! input frontends */
const frontend_input *frontend_set_input[] =
{
	&null_frontend_input,
#ifdef HAVE_LIBCACA
	&libcaca_frontend_input,
#else
	&null_frontend_input,
#endif
#ifdef HAVE_COMPILER_MSVC
	&w32_frontend_input,
#else
	&null_frontend_input,
#endif
#ifdef HAVE_SDL2
	&sdl2_frontend_input,
#else
	&null_frontend_input,
#endif
};

/*! Video frontends */
const frontend_video *frontend_set_video[] =
{
	&null_frontend_video,
#ifdef HAVE_LIBCACA
	&libcaca_frontend_video,
#else
	&null_frontend_video,
#endif
#ifdef HAVE_COMPILER_MSVC
	&w32_frontend_video,
#else
	&null_frontend_video,
#endif
#ifdef HAVE_SDL2
	&sdl2_frontend_video,
#else
	&null_frontend_video,
#endif
};

/*! Audio frontends */
const frontend_audio *frontend_set_audio[] =
{
	&null_frontend_audio,
	&null_frontend_audio,
	&null_frontend_audio,
#ifdef HAVE_SDL2
	&sdl2_frontend_audio,
#else
	&null_frontend_audio,
#endif
};

/*! Event loops */
const frontend_event_loop frontend_set_event_loop[] =
{
	&null_event_loop,
#ifdef HAVE_LIBCACA
	&libcaca_event_loop,
#else
	&null_event_loop,
#endif
#ifdef HAVE_COMPILER_MSVC
	&w32_event_loop,
#else
	&null_event_loop,
#endif
#ifdef HAVE_SDL2
	&sdl2_event_loop,
#else
	&null_event_loop,
#endif
};
