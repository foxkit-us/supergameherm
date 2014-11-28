#include "config.h"	// macros

#include "print.h"	// fatal
#include "ctl_unit.h"	// signal_interrupt
#include "util.h"	// likely/unlikely
#include "sgherm.h"	// emu_state


#define LCDC_BGWINDOW_SHOW	0x01
#define LCDC_OBJ_DISPLAY	0x02
#define LCDC_OBJ_LGSIZE		0x04
#define LCDC_BGTILE_MAP_HI	0x08
#define LCDC_BGWINDOW_TILE_LO	0x10
#define LCDC_WINDOW_SHOW	0x20
#define LCDC_WINTILE_MAP_HI	0x40
#define LCDC_ENABLE		0x80

void init_lcdc(emu_state *restrict state)
{
	state->memory[0xFF40] = LCDC_ENABLE | LCDC_BGWINDOW_TILE_LO | LCDC_BGWINDOW_SHOW;
	state->memory[0xFF41] = 0x02;
	state->memory[0xFF42] = 0x00;
	state->memory[0xFF43] = 0x00;
	state->memory[0xFF44] = 0x00;
	state->memory[0xFF45] = 0x00;
}

static inline void _lcdc_inc_mode(emu_state *restrict state)
{
	uint8_t *byte = state->memory + 0xFF41;

	if((*byte & 0x3) == 0x3)
	{
		*byte &= ~0x3;
	}
	else
	{
		*byte += 1;
	}
}

void lcdc_tick(emu_state *restrict state)
{
	uint32_t *clk = &(state->lcdc_state.curr_clk);
	uint8_t *ly = state->memory + 0xFF44;
	uint8_t curr_mode = state->memory[0xFF41] & 0x3;

	*clk += 1;

	if(unlikely(state->stop))
	{
		return;
	}

	switch(curr_mode)
	{
	case 2:
		/* first mode - reading OAM for h scan line */
		if(*clk >= 80)
		{
			*clk = 0;
			_lcdc_inc_mode(state);
		}
		break;
	case 3:
		/* second mode - reading VRAM for h scan line */
		if(*clk >= 172)
		{
			*clk = 0;
			_lcdc_inc_mode(state);
		}
		break;
	case 0:
		/* third mode - h-blank */
		if(*clk >= 204)
		{
			*clk = 0;
			*ly += 1;
			if(*ly == 144)
			{
				/* going to v-blank */
				_lcdc_inc_mode(state);
			}
			else
			{
				/* start another scan line */
				state->memory[0xFF41] |= 0x02;
			}
		}
		break;
	case 1:
		/* v-blank */
		if(*ly == 144 && *clk == 1)
		{
			// Fire the vblank interrupt
			signal_interrupt(state, INT_VBLANK);
		}

		if(*clk % 456 == 0)
		{
			*ly += 1;
		};

		if(*ly == 153)
		{
			*clk = 0;
			*ly = 0;
			_lcdc_inc_mode(state);
		}

		break;
	default:
		fatal("somehow wound up in an unknown impossible video mode");
	}
}

uint8_t lcdc_read(emu_state *restrict state, uint16_t reg)
{
	if(!(reg & 0xFF00))
	{
		/* this is raw video RAM read */
		uint8_t curr_mode = state->memory[0xFF41] & 0x3;
		if(curr_mode > 1)
		{
			fatal("read from VRAM while not in h/v-blank");
			return 0xFF;
		}
	}
	/* XXX TODO FIXME */
	return state->memory[reg];
}

void lcdc_write(emu_state *restrict state, uint16_t reg, uint8_t data)
{
	if(reg == 0xFF44)
	{
#ifndef NDEBUG
		fatal("write to LY (FF44); you can't just vsync yourself!");
#else
		error("write to LY (FF44) is being ignored");
#endif
		return;
	}

	/* XXX TODO FIXME */
	state->memory[reg] = data;
}

uint8_t bg_pal_ind_read(emu_state *restrict state, uint16_t reg)
{
	if(state->system != SYSTEM_CGB)
	{
		return no_hardware(state, reg);
	}

	// TODO
	return state->memory[reg];
}

uint8_t bg_pal_data_read(emu_state *restrict state, uint16_t reg)
{
	if(state->system != SYSTEM_CGB)
	{
		return no_hardware(state, reg);
	}

	// TODO
	return state->memory[reg];
}

uint8_t sprite_pal_ind_read(emu_state *restrict state, uint16_t reg)
{
	if(state->system != SYSTEM_CGB)
	{
		return no_hardware(state, reg);
	}

	// TODO
	return state->memory[reg];
}

uint8_t sprite_pal_data_read(emu_state *restrict state, uint16_t reg)
{
	if(state->system != SYSTEM_CGB)
	{
		return no_hardware(state, reg);
	}

	// TODO
	return state->memory[reg];
}

void bg_pal_ind_write(emu_state *restrict state, uint16_t reg, uint8_t data)
{
	if(state->system != SYSTEM_CGB)
	{
		return doofus_write(state, reg, data);
	}

	// TODO
	state->memory[reg] = data;
}

void bg_pal_data_write(emu_state *restrict state, uint16_t reg, uint8_t data)
{
	if(state->system != SYSTEM_CGB)
	{
		return doofus_write(state, reg, data);
	}

	// TODO
	state->memory[reg] = data;
}

void sprite_pal_ind_write(emu_state *restrict state, uint16_t reg, uint8_t data)
{
	if(state->system != SYSTEM_CGB)
	{
		return doofus_write(state, reg, data);
	}

	// TODO
	state->memory[reg] = data;
}

void sprite_pal_data_write(emu_state *restrict state, uint16_t reg, uint8_t data)
{
	if(state->system != SYSTEM_CGB)
	{
		return doofus_write(state, reg, data);
	}

	// TODO
	state->memory[reg] = data;
}

