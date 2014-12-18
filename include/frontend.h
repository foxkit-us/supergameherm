#ifndef __FRONTEND_H__
#define __FRONTEND_H__

#include "config.h"	// bool
#include "typedefs.h"	// typedefs
#include "input.h"	// input_key

struct frontend_input_return_t
{
	input_key key;
	bool press;
};

struct frontend_input_t
{
	bool (*init)(emu_state *);		//! Initalise the keyboard input
	void (*finish)(emu_state *);		//! Deinitalise the keyboard input
	void (*get_key)(emu_state *, frontend_input_return *);		//! Get a key

	void *data;				//! Opaque data
};

struct frontend_audio_t
{
	bool (*init)(emu_state *);		//! Initalise the audio output
	void (*finish)(emu_state *);		//! Deinitalise the audio output
	void (*output_sample)(emu_state *);	//! Output an audio sample

	void *data;				//! Opaque data
};

struct frontend_video_t
{
	bool (*init)(emu_state *);		//! Initalise the video output
	void (*finish)(emu_state *);		//! Deinitalise the video output
	void (*blit_canvas)(emu_state *);	//! Blit the canvas

	void *data;				//! Opaque data
};

struct frontend_t
{
	frontend_input input;
	frontend_audio audio;
	frontend_video video;

	bool input_set, audio_set, video_set;

	int (*event_loop)(emu_state *);		//! Event loop function (for use with toolkits)

	void *data;				//! Opaque data
};

//! Frontend indicies
typedef enum
{
	FRONT_NULL = 0,
	FRONT_LIBCACA = 1,
	FRONT_WIN32 = 2,
	FRONT_SDL2 = 3,
} frontend_type;


//! Change the frontend
bool select_frontend_input(emu_state * restrict, const frontend_input * restrict);
bool select_frontend_audio(emu_state * restrict, const frontend_audio * restrict);
bool select_frontend_video(emu_state * restrict, const frontend_video * restrict);
bool select_frontend_all(emu_state * restrict, const frontend_input * restrict,
	const frontend_audio * restrict, const frontend_video * restrict,
	int (*)(emu_state *));
void finish_frontend(emu_state *restrict);

//! Null frontends
extern const frontend_input null_frontend_input;
extern const frontend_audio null_frontend_audio;
extern const frontend_video null_frontend_video;
int null_event_loop(emu_state *);

#define NULL_INPUT &null_frontend_input
#define NULL_AUDIO &null_frontend_audio
#define NULL_VIDEO &null_frontend_video
#define NULL_LOOP &null_event_loop

//! libcaca frontends
#ifdef HAVE_LIBCACA
#	include "frontends/caca/frontend.h"
#	define LIBCACA_INPUT &libcaca_frontend_input
#	define LIBCACA_AUDIO &null_frontend_audio
#	define LIBCACA_VIDEO &libcaca_frontend_video
#	define LIBCACA_LOOP &libcaca_event_loop
#else
#	define LIBCACA_INPUT &null_frontend_input
#	define LIBCACA_AUDIO &null_frontend_audio
#	define LIBCACA_VIDEO &null_frontend_video
#	define LIBCACA_LOOP &null_event_loop
#endif

//! SDL2 frontends
#ifdef HAVE_SDL2
#	include "frontends/sdl2/frontend.h"
#	define SDL2_INPUT &sdl2_frontend_input
#	define SDL2_AUDIO &sdl2_frontend_audio
#	define SDL2_VIDEO &sdl2_frontend_video
#	define SDL2_LOOP sdl2_event_loop
#else
#	define SDL2_INPUT &null_frontend_input
#	define SDL2_AUDIO &null_frontend_audio
#	define SDL2_VIDEO &null_frontend_video
#	define SDL2_LOOP &null_event_loop
#endif

//! Win32 frontend
#ifdef HAVE_WINDOWS
#	include "frontends/w32/frontend.h"
#	define WIN32_INPUT &w32_frontend_input
#	define WIN32_AUDIO &null_frontend_audio
#	define WIN32_VIDEO &w32_frontend_video
#	define WIN32_LOOP &w32_event_loop
#else
#	define WIN32_INPUT &null_frontend_input
#	define WIN32_AUDIO &null_frontend_audio
#	define WIN32_VIDEO &null_frontend_video
#	define WIN32_LOOP &null_event_loop
#endif

// Helpers to call functions
#define CALL_FRONTEND_0(state, type, fn) ((*(state->front.type.fn))(state))
#define CALL_FRONTEND_1(state, type, fn, _1) ((*(state->front.type.fn))(state, _1))
#define EVENT_LOOP(state) ((*(state->front.event_loop))(state))

#define FRONTEND_INIT_INPUT(state) CALL_FRONTEND_0(state, input, init)
#define FRONTEND_INIT_AUDIO(state) CALL_FRONTEND_0(state, audio, init)
#define FRONTEND_INIT_VIDEO(state) CALL_FRONTEND_0(state, video, init)
#define FRONTEND_FINISH_INPUT(state) CALL_FRONTEND_0(state, input, finish)
#define FRONTEND_FINISH_AUDIO(state) CALL_FRONTEND_0(state, audio, finish)
#define FRONTEND_FINISH_VIDEO(state) CALL_FRONTEND_0(state, video, finish)

#define BLIT_CANVAS(state) CALL_FRONTEND_0(state, video, blit_canvas)
#define OUTPUT_SAMPLE(state) CALL_FRONTEND_0(state, audio, output_sample)
#define GET_KEY(state, ret) CALL_FRONTEND_1(state, input, get_key, ret)

#endif /*__FRONTEND_H__*/
