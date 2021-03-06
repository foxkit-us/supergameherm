#ifndef __SOUND_H_
#define __SOUND_H_

#include "config.h"	// Various macros, uint[XX]_t
#include "typedefs.h"	// typedefs


struct snd_state_t
{
	int per_env; //! Counter for envelope update period
	int freq; //! Audio output frequency
	int freq_rem; //! Counter for period skip calculation

	struct _ch1
	{
		bool enabled;		//! channel enabled?
		bool initial;		//! channel started?
		uint8_t sweep_time;	//! this/128Hz = sweep
		/*! if true, sweep increases period.
		 *  otherwise, sweep decreases period. */
		bool sweep_dec;
		uint8_t sweep_shift;	//! number of shift
		uint8_t wave_duty;	//! wave pattern duty: 1=12.5%,2=25%,3=50%,4=75%
		uint8_t length;		//! sound length
		int8_t envelope_volume;	//! initial envelope volume
		bool envelope_amp;	//! determines if envelope amplifies (c.f. attenuates)
		uint8_t envelope_speed;	//! speed of envelope
		bool counter;		//! if true, one-shot.  otherwise, loop
		uint16_t period;	//! 11-bit period (higher 5 = nothing)
		bool s01;		//! output to S01
		bool s02;		//! output to S02

		// Audio runtime values
		uint16_t per_remain;	//! Counter for period
		uint8_t outseq;		//! Output sequence number for wave duty
	} ch1;

	struct _ch2
	{
		bool enabled;		//! channel enabled?
		bool initial;		//! channel started?
		uint8_t wave_duty;	//! wave pattern duty: 1=12.5%,2=25%,3=50%,4=75%
		uint8_t length;		//! sound length
		int8_t envelope_volume;	//! initial envelope volume
		bool envelope_amp;	//! determines if envelope amplifies (c.f. attenuates)
		uint8_t envelope_speed;	//! speed of envelope
		bool counter;		//! if true, one-shot.  otherwise, loop
		uint16_t period;	//! 11-bit period (higher 5 = nothing)
		bool s01;		//! output to S01
		bool s02;		//! output to S02

		// Audio runtime values
		uint16_t per_remain;	//! Counter for period
		uint8_t outseq;		//! Output sequence number for wave duty
	} ch2;

	struct _ch3
	{
		bool enabled;		//! channel enabled?
		bool initial;		//! channel started?
		bool counter;		//! if true, one-shot.  otherwise, loop
		uint8_t length;		//! sound length
		uint8_t volume;		//! sound volume value
		uint16_t period;	//! 11-bit period (higher 5 = nothing)
		uint8_t wave[16];	//! waveform data
		bool s01;		//! output to S01
		bool s02;		//! output to S02

		// Audio runtime values
		uint16_t per_remain;	//! Counter for period
		uint8_t outseq;		//! Output sequence number for waveform
	} ch3;

	struct _ch4
	{
		bool enabled;		//! channel enabled?
		bool initial;		//! channel started?
		uint8_t length;		//! sound length
		int8_t envelope_volume;	//! initial envelope volume
		bool envelope_amp;	//! determines if envelope amplifies (c.f. attenuates)
		uint8_t envelope_speed;	//! speed of envelope
		uint8_t period_exp;	//! exponential part of period
		uint8_t period_mul;	//! (mostly) linear part of period
		bool is_short;		//! determines if LFSR is 7-bit (c.f. 15-bit)
		bool counter;		//! if true, one-shot.  otherwise, loop
		bool s01;		//! output to S01
		bool s02;		//! output to S02

		// Audio runtime values
		int32_t per_remain;	//! Counter for period
		uint16_t lfsr;		//! LFSR counter data
	} ch4;

	bool enabled;			//! sound active?
	bool s01;			//! S01 enabled?
	uint8_t s01_volume;		//! S01 volume
	bool s02;			//! S02 enabled?
	uint8_t s02_volume;		//! S02 volume
};


void sound_fetch_s16ne(emu_state *restrict, int16_t *restrict, size_t);
void sound_tick(emu_state *restrict, int);

#endif /*!__SOUND_H_*/
