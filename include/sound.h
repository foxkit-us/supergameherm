#ifndef __SOUND_H_
#define __SOUND_H_

#include "config.h"	// Various macros, uint[XX]_t
#include "typedefs.h"	// typedefs


struct snd_state_t
{
	int per_env; // Counter for envelope update period

	struct _ch1
	{
		bool enabled; //! channel enabled?
		uint8_t sweep_time; //! this/128Hz = sweep
		/*! if true, sweep increases period.
		 *  otherwise, sweep decreases period. */
		bool sweep_dec;
		uint8_t sweep_shift; //! number of shift
		uint8_t wave_duty; //! wave pattern duty: 1=12.5%,2=25%,3=50%,4=75%
		uint8_t length; //! sound length
		int8_t envelope_volume; //! initial envelope volume
		/*! if true, envelope amplifies.
		 *  otherwise, envelope attenuates. */
		bool envelope_amp;
		uint8_t envelope_speed; //! speed of envelope
		bool counter; //! if true, one-shot.  otherwise, loop
		uint16_t period; //! 11-bit period (higher 5 = nothing)
		bool s01; //! output to S01
		bool s02; //! output to S02

		// Audio runtime values
		uint16_t per_remain; //! Counter for period
		uint8_t outseq; //! Output sequence number for wave duty
	} ch1;

	struct _ch2
	{
		bool enabled; //! channel enabled?
		uint8_t wave_duty; //! wave pattern duty: 1=12.5%,2=25%,3=50%,4=75%
		uint8_t length; //! sound length
		int8_t envelope_volume; //! initial envelope volume
		/*! if true, envelope amplifies.
		 *  otherwise, envelope attenuates. */
		bool envelope_amp;
		uint8_t envelope_speed; //! speed of envelope
		bool counter; //! if true, one-shot.  otherwise, loop
		uint16_t period; //! 11-bit period (higher 5 = nothing)
		bool s01; //! output to S01
		bool s02; //! output to S02

		// Audio runtime values
		uint16_t per_remain; //! Counter for period
		uint8_t outseq; //! Output sequence number for wave duty
	} ch2;

	struct _ch3
	{
		bool enabled;		//! channel enabled?
		uint8_t wave[16];	//! waveform data
		bool s01;		//! output to S01
		bool s02;		//! output to S02
	} ch3;

	struct _ch4
	{
		bool enabled;		//! channel enabled?
		bool s01;		//! output to S01
		bool s02;		//! output to S02
	} ch4;

	bool enabled;			//! sound active?
	bool s01;			//! S01 enabled?
	uint8_t s01_volume;		//! S01 volume
	bool s02;			//! S02 enabled?
	uint8_t s02_volume;		//! S02 volume
};


uint8_t sound_read(emu_state *restrict, uint16_t);
void sound_write(emu_state *restrict, uint16_t, uint8_t);
void sound_tick(emu_state *restrict);

#endif /*!__SOUND_H_*/
