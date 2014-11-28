#ifndef __UTIL_H__
#define __UTIL_H__

#include "config.h"		// macros, bool, uint[XX]_t

// Functions
uint32_t interleave(uint32_t);
void interleaved_to_buf(uint32_t, uint8_t *);

#endif /*__UTIL_H__*/
