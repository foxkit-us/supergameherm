#ifndef __TIMER_H_
#define __TIMER_H_

#include <stdio.h>	// perror
#include <time.h>	// nanosleep, timespec

#include "print.h"	// fatal
#include "sgherm.h"	// emulator_state

#include <stdint.h>	// uint[XX]_t


uint8_t timer_read(emulator_state *restrict, uint16_t);
void timer_write(emulator_state *restrict, uint16_t, uint8_t);
void timer_tick(emulator_state *restrict);

#endif /*!__TIMER_H_*/
