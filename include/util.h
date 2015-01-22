#ifndef __UTIL_H__
#define __UTIL_H__

#include "config.h"		// macros, bool, uint[XX]_t

typedef struct time_delta_t
{
	uint8_t seconds;
	uint8_t minutes;
	uint8_t hours;
	uint64_t days;

	bool negative;
} time_delta;

// Functions
uint32_t interleave8(uint8_t, uint8_t, uint8_t, uint8_t);
uint32_t interleave16(uint16_t, uint16_t);
int get_file_size(const char *);

void unix_time_delta(uint64_t, uint64_t, time_delta *);

#endif /*__UTIL_H__*/
