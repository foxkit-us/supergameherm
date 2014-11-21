#include "config.h"	// macros
#include "print.h"	// fatal
#include "sgherm.h"	// emulator_state


#define LCDC_BGWINDOW_SHOW	0x01
#define LCDC_OBJ_DISPLAY	0x02
#define LCDC_OBJ_LGSIZE		0x04
#define LCDC_BGTILE_MAP_HI	0x08
#define LCDC_BGWINDOW_TILE_LO	0x10
#define LCDC_WINDOW_SHOW	0x20
#define LCDC_WINTILE_MAP_HI	0x40
#define LCDC_ENABLE		0x80

void init_lcdc(emulator_state *restrict state)
{
	state->memory[0xFF40] = LCDC_ENABLE | LCDC_BGWINDOW_TILE_LO | LCDC_BGWINDOW_SHOW;
	state->memory[0xFF41] = 0x02;
	state->memory[0xFF42] = 0x00;
	state->memory[0xFF43] = 0x00;
	state->memory[0xFF44] = 0x00;
	state->memory[0xFF45] = 0x00;
}

static inline void _lcdc_inc_mode(emulator_state *restrict state)
{
	uint8_t *byte = state->memory + 0xFF41;
	if(*byte & 0x3)
	{
		*byte &= 0x78;
	}
	else
	{
		*byte += 1;
	}
}

void lcdc_tick(emulator_state *restrict state)
{
	uint32_t *clk = &(state->lcdc_state.curr_clk);
	*clk += 1;
	uint8_t *ly = state->memory + 0xFF44;
	uint8_t curr_mode = state->memory[0xFF41] & 0x3;

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
		if(*clk % 456 == 0)
		{
			*ly += 1;
		};

		if(*ly == 153)
		{
			*clk = 0;
			_lcdc_inc_mode(state);
		}
		break;
	default:
		fatal("somehow wound up in an unknown impossible video mode");
	}
}

uint8_t lcdc_read(emulator_state *restrict state, uint16_t reg)
{
	/* XXX TODO FIXME */
	return state->memory[reg];
}

void lcdc_write(emulator_state *restrict state, uint16_t reg, uint8_t data)
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
