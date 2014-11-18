#ifndef __TIMER_H_
#define __TIMER_H_

#include "sgherm.h"	// emulator_state

void init_clock(emulator_state *);
void clock_tick(emulator_state *);

/* TODO GB hardware timers */

#endif /*!__TIMER_H_*/
