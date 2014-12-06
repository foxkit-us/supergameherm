#ifndef __TYPEDEFS_H__
#define __TYPEDEFS_H__

typedef struct frontend_t frontend;
typedef struct frontend_audio_t frontend_audio;
typedef struct frontend_video_t frontend_video;
typedef struct frontend_input_t frontend_input;

typedef struct oam_t oam;
typedef struct cps_t cps;

typedef struct emu_state_t emu_state;
typedef struct interrupt_state_t interrupt_state;
typedef struct input_state_t input_state;
typedef struct lcdc_state_t lcdc_state;
typedef struct cart_header_t cart_header;
typedef struct ser_state_t ser_state;
typedef struct registers_t register_state;
typedef struct snd_state_t snd_state;
typedef struct timer_state_t timer_state;

// Depends on emu_state *sigh*
typedef int (*frontend_event_loop)(emu_state *);

#endif /*__TYPEDEFS_H__*/
