#ifndef __MEMORY_H_
#define __MEMORY_H_

#include <stdint.h>	/* (u)int*_t */
#include <stdbool.h>	/* bool */
#include <stdio.h>	/* FILE * */

#include "sgherm.h"	/* emulator_state */
#include "params.h"	/* system_types */


uint8_t mem_read8(emulator_state *restrict, uint16_t);
uint16_t mem_read16(emulator_state *restrict, uint16_t);
void mem_write8(emulator_state *restrict, uint16_t, uint8_t);
void mem_write16(emulator_state *restrict, uint16_t, uint16_t);

#endif /*!__MEMORY_H_*/
