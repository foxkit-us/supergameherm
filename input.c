#include <assert.h>	/* assert */
#include <stdint.h>	/* uint[XX]_t */

#include "print.h"	/* error */
#include "sgherm.h"	/* emulator_state */


uint8_t joypad_read(emulator_state *restrict state unused, uint16_t reg)
{
	assert(reg == 0xFF00);

	if(reg != 0xFF00)
	{
		error("joypad: unknown register %04X (R)", reg);
		return -1;
	}

	/* TODO write an input engine, probably with SDL. */
	return 0;
}

void joypad_write(emulator_state *restrict state unused, uint16_t reg unused, uint8_t data unused)
{
	/* TODO see above */
	return;
}
