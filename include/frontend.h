#ifndef __FRONTEND_H__
#define __FRONTEND_H__

#include "config.h"	// bool
#include "typedefs.h"	// typedefs
#include "input.h"	// input_key


struct frontend_input_t
{
	bool (*init)(emu_state *);		/*! Initalise the keyboard input */
	void (*finish)(emu_state *);		/*! Deinitalise the keyboard input */
	input_key (*get_key)(emu_state *);	/*! Get a key */

	void *data;				/*! Opaque data */
};

struct frontend_audio_t
{
	bool (*init)(emu_state *);		/*! Initalise the audio output */
	void (*finish)(emu_state *);		/*! Deinitalise the audio output */
	void (*output_sample)(emu_state *);	/*! Output an audio sample */

	void *data;				/*! Opaque data */
};

struct frontend_video_t
{
	bool (*init)(emu_state *);		/*! Initalise the video output */
	void (*finish)(emu_state *);		/*! Deinitalise the video output */
	void (*blit_canvas)(emu_state *);	/*! Blit the canvas */

	void *data;				/*! Opaque data */
};

struct frontend_t
{
	frontend_input input;
	frontend_audio audio;
	frontend_video video;

	int (*event_loop)(emu_state *);		/*! Event loop function (for use with toolkits) */

	void *data;				/*! Opaque data */
};

/*! Null frontends */
extern const frontend_input null_frontend_input;
extern const frontend_audio null_frontend_audio;
extern const frontend_video null_frontend_video;
int null_event_loop(emu_state *);

/*! libcaca frontends */
#ifdef HAVE_LIBCACA
extern const frontend_input libcaca_frontend_input;
extern const frontend_video libcaca_frontend_video;
int libcaca_event_loop(emu_state *);
#endif

/*! Win32 frotnend */
#ifdef HAVE_COMPILER_MSVC	/* XXX - won't work on msys */
extern const frontend_input w32_frontend_input;
extern const frontend_video w32_frontend_video;
int w32_event_loop(emu_state *);
#endif

/*! Frontend indicies */
typedef enum
{
	FRONT_NULL = 0,
	FRONT_LIBCACA = 1,
	FRONT_WIN32 = 2,
} frontend_type;

extern const frontend_input *frontend_set_input[];
extern const frontend_video *frontend_set_video[];
extern const frontend_audio *frontend_set_audio[];
extern const frontend_event_loop frontend_set_event_loop[];

// Helpers to call functions
#define CALL_FRONTEND(state, type, fn) ((*(state->front.type.fn))(state))
#define EVENT_LOOP(state) ((*(state->front.event_loop))(state))

#define FRONTEND_INIT_INPUT(state) CALL_FRONTEND(state, input, init)
#define FRONTEND_INIT_AUDIO(state) CALL_FRONTEND(state, audio, init)
#define FRONTEND_INIT_VIDEO(state) CALL_FRONTEND(state, video, init)
#define FRONTEND_FINISH_INPUT(state) CALL_FRONTEND(state, input, finish)
#define FRONTEND_FINISH_AUDIO(state) CALL_FRONTEND(state, audio, finish)
#define FRONTEND_FINISH_VIDEO(state) CALL_FRONTEND(state, video, finish)

#define FRONTEND_INIT_ALL(state) \
	{ \
		FRONTEND_INIT_INPUT(state); \
		FRONTEND_INIT_AUDIO(state); \
		FRONTEND_INIT_VIDEO(state); \
	}

#define FRONTEND_FINISH_ALL(state) \
	{ \
		FRONTEND_FINISH_INPUT(state); \
		FRONTEND_FINISH_AUDIO(state); \
		FRONTEND_FINISH_VIDEO(state); \
	}

#define BLIT_CANVAS(state) CALL_FRONTEND(state, video, blit_canvas)
#define OUTPUT_SAMPLE(state) CALL_FRONTEND(state, audio, output_sample)
#define GET_KEY(state) CALL_FRONTEND(state, input, get_key)

#endif /*__FRONTEND_H__*/
