#ifndef __SGH_SIGNALS_H__
#define __SGH_SIGNALS_H__

extern volatile bool do_exit;

extern emu_state *state_current;


void register_handlers(void);

#endif
