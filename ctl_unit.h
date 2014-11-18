#ifndef __CTL_UNIT_H_
#define __CTL_UNIT_H_

#include "sgherm.h"	// emulator_state

void init_ctl(emulator_state *, char);
bool execute(emulator_state *);

#endif /*!__CTL_UNIT_H_*/
