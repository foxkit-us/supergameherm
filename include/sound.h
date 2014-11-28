#ifndef __SOUND_H_
#define __SOUND_H_

#include "config.h"	// Various macros, uint[XX]_t


typedef struct _snd_state
{
	struct _ch1
	{
		/*! channel enabled? */
		bool enabled;
		/*! this/128Hz = sweep */
		uint8_t sweep_time;
		/*! if true, sweep decreases frequency.
		 *  otherwise, sweep increases frequency. */
		bool sweep_dec;
		/*! number of shift */
		uint8_t shift;
		/*! wave pattern duty: 1=12.5%,2=25%,3=50%,4=75% */
		uint8_t wave_duty;
		/*! sound length */
		uint8_t length;
		/*! initial envelope volume */
		uint8_t envelope_volume;
		/*! if true, envelope amplifies.
		 *  otherwise, envelope attenuates. */
		bool envelope_amp;
		/*! number of sweeps (0 = stop) */
		uint8_t sweep;
		/*! if true, one-shot.  otherwise, loop */
		bool counter;
		/*! 11-bit frequency (higher 5 = nothing) */
		uint16_t frequency;
	} ch1;
	struct _ch2
	{
		bool enabled;		/*! channel enabled? */
	} ch2;
	struct _ch3
	{
		bool enabled;		/*! channel enabled? */
		uint8_t wave[16];	/*! waveform data */
	} ch3;
	struct _ch4
	{
		bool enabled;		/*! channel enabled? */
	} ch4;
	bool enabled;			/*! sound active? */
} snd;


typedef struct _emu_state emu_state;

uint8_t sound_read(emu_state *restrict, uint16_t);
void sound_write(emu_state *restrict, uint16_t, uint8_t);
void sound_tick(emu_state *restrict);

#endif /*!__SOUND_H_*/
