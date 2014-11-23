#ifndef __TIMER_H_
#define __TIMER_H_

#include "config.h"	// Various macros

#include <stdio.h>	// perror
#include <stdint.h>	// uint[XX]_t

#include "util.h"	// likely/unlikely


typedef struct _tm_state
{
	uint8_t div;			/*! DIV register */
	uint8_t tima;			/*! TIMA register */
	uint8_t rounds;			/*! TMA register */
	uint16_t ticks_per_tima;	/*! ticks per TIMA++ */
	uint8_t curr_clk;		/*! ticks passed */
	bool enabled;			/*! timer armed */
} timer;

typedef enum
{
	CPU_FREQ_GB = 4194304,
	CPU_FREQ_SGB = 4295454,
	CPU_FREQ_GBC = 8388608,
} cpu_freq;

typedef struct _emu_state emu_state;

uint8_t timer_read(emu_state *restrict, uint16_t);
void timer_write(emu_state *restrict, uint16_t, uint8_t);
void timer_tick(emu_state *restrict);

#endif /*!__TIMER_H_*/
