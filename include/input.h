#ifndef __INPUT_H_
#define __INPUT_H_

#include "config.h"	// macros, uint[XX]_t


#define RAW_INPUT_P10	0x1
#define RAW_INPUT_P11	0x2
#define RAW_INPUT_P12	0x4
#define RAW_INPUT_P13	0x8

// Columns set
#define RAW_INPUT_P14	0x10
#define RAW_INPUT_P15	0x20

typedef enum
{
	INPUT_RIGHT = (RAW_INPUT_P10 | RAW_INPUT_P14),
	INPUT_LEFT = (RAW_INPUT_P11 | RAW_INPUT_P14),
	INPUT_UP = (RAW_INPUT_P12 | RAW_INPUT_P14),
	INPUT_DOWN = (RAW_INPUT_P13 | RAW_INPUT_P14),
	INPUT_A = (RAW_INPUT_P10 | RAW_INPUT_P15),
	INPUT_B = (RAW_INPUT_P11 | RAW_INPUT_P15),
	INPUT_SELECT = (RAW_INPUT_P12 | RAW_INPUT_P15),
	INPUT_START = (RAW_INPUT_P13 | RAW_INPUT_P15),
} input_key;

typedef struct _input_state
{
	uint8_t col_state;		/*! P14 and P15 */
	uint8_t row_state;		/*! P10 through P13 */
} input;


typedef struct _emu_state emu_state;

uint8_t joypad_read(emu_state *restrict, uint16_t);
void joypad_write(emu_state *restrict, uint16_t, uint8_t);
void joypad_signal(emu_state *restrict, input_key, bool);

#endif /*!__INPUT_H_*/
