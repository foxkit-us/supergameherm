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
	state->lcdc.lcd_control.params.enable = true;
	state->lcdc.lcd_control.params.bg_char_sel = true;
	state->lcdc.lcd_control.params.dmg_bg = true;

	state->lcdc.stat.params.mode_flag = 2;

	state->lcdc.ly = 0;
	state->lcdc.lyc = 0;
}

static inline void _lcdc_inc_mode(emu_state *restrict state)
{
	state->lcdc.stat.params.mode_flag++;
}

void lcdc_tick(emu_state *restrict state)
{
	state->lcdc.curr_clk++;

	if(unlikely(state->stop))
	{
		return;
	}

	switch(state->lcdc.stat.params.mode_flag)
	{
	case 2:
		/* first mode - reading OAM for h scan line */
		if(state->lcdc.curr_clk >= 80)
		{
			state->lcdc.curr_clk = 0;
			_lcdc_inc_mode(state);
		}
		break;
	case 3:
		/* second mode - reading VRAM for h scan line */
		if(state->lcdc.curr_clk >= 172)
		{
			state->lcdc.curr_clk = 0;
			_lcdc_inc_mode(state);
		}
		break;
	case 0:
		/* third mode - h-blank */
		if(state->lcdc.curr_clk >= 204)
		{
			state->lcdc.curr_clk = 0;
			if((++state->lcdc.ly) == 144)
			{
				/* going to v-blank */
				_lcdc_inc_mode(state);
			}
			else
			{
				/* start another scan line */
				state->lcdc.stat.params.mode_flag = 2;
			}
		}
		break;
	case 1:
		/* v-blank */
		if(state->lcdc.ly == 144 &&
		   state->lcdc.curr_clk == 1)
		{
			// Fire the vblank interrupt
			signal_interrupt(state, INT_VBLANK);
		}

		if(state->lcdc.curr_clk % 456 == 0)
		{
			state->lcdc.ly++;
		};

		if(state->lcdc.ly == 153)
		{
			state->lcdc.curr_clk = 0;
			state->lcdc.ly = 0;
			_lcdc_inc_mode(state);
		}

		break;
	default:
		fatal("somehow wound up in an unknown impossible video mode");
	}
	
	if(state->lcdc.ly == state->lcdc.lyc)
	{
		state->lcdc.stat.params.lyc_state = true;
		if(state->lcdc.stat.params.lyc)
		{
			signal_interrupt(state, INT_LCD_STAT);
		}
	}
}

uint8_t lcdc_read(emu_state *restrict state, uint16_t reg)
{
	if(!(reg & 0xFF00))
	{
		/* this is raw video RAM read */
		uint8_t curr_mode = state->lcdc.stat.params.mode_flag;
		if(curr_mode > 1)
		{
			fatal("read from VRAM while not in h/v-blank");
			return 0xFF;
		}
		return state->memory[reg];
	}
	switch(reg)
	{
	case 0xFF40:
		return state->lcdc.lcd_control.reg;
	case 0xFF41:
		return state->lcdc.stat.reg;
	case 0xFF44:
		return state->lcdc.ly;
	case 0xFF45:
		return state->lcdc.lyc;
	default:
		error("lcdc: unknown register %04X (R)", reg);
		return 0xFF;
	}
}

void lcdc_write(emu_state *restrict state, uint16_t reg, uint8_t data)
{
	switch(reg)
	{
	case 0xFF40:
		state->lcdc.lcd_control.reg = data;
		break;
	case 0xFF41:
		state->lcdc.stat.reg = data;
		break;
	case 0xFF44:
#ifndef NDEBUG
		fatal("write to LY (FF44); you can't just vsync yourself!");
#else
		error("write to LY (FF44) is being ignored");
#endif
		break;
	case 0xFF45:
		state->lcdc.lyc = data;
		break;
	default:
		error("lcdc: unknown register %04X (W)", reg);
	}
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

