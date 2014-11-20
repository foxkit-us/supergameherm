#ifndef __MEMORY_H_
#define __MEMORY_H_

#include <stdint.h>	/* (u)int*_t */
#include <stdbool.h>	/* bool */
#include <stdio.h>	/* FILE * */

#include "sgherm.h"	/* emulator_state */
#include "params.h"	/* system_types */


uint8_t mem_read8(emulator_state *state, uint16_t location);
uint16_t mem_read16(emulator_state *state, uint16_t location);
void mem_write8(emulator_state *state, uint16_t location, uint8_t data);
void mem_write16(emulator_state *state, uint16_t location, uint16_t data);

#endif /*!__MEMORY_H_*/
