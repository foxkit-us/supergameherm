#include "config.h"	// macros

#include "print.h"	// fatal
#include "ctl_unit.h"	// signal_interrupt
#include "util.h"	// likely/unlikely
#include "sgherm.h"	// emu_state

#include <assert.h>
#include <stdlib.h>	// abs


#define LCDC_BGWINDOW_SHOW	0x01
#define LCDC_OBJ_DISPLAY	0x02
#define LCDC_OBJ_LGSIZE		0x04
#define LCDC_BGTILE_MAP_HI	0x08
#define LCDC_BGWINDOW_TILE_LO	0x10
#define LCDC_WINDOW_SHOW	0x20
#define LCDC_WINTILE_MAP_HI	0x40
#define LCDC_ENABLE		0x80

uint32_t dmg_palette[4] = { 0x00FFFFFF, 0x00AAAAAA, 0x00777777, 0x00000000 };

void init_lcdc(emu_state *restrict state)
{
	state->lcdc.lcd_control.enable = true;
	state->lcdc.lcd_control.bg_char_sel = true;
	state->lcdc.lcd_control.dmg_bg = true;

	state->lcdc.stat.mode_flag = 2;

	state->lcdc.ly = 0;
	state->lcdc.lyc = 0;
}

static inline void dmg_bg_render(emu_state *restrict state)
{
	// Compute positions in the "virtual" map of tiles
	const uint8_t sy = state->lcdc.ly + state->lcdc.scroll_y, s_sy = sy / 8;
	uint8_t x, sx = state->lcdc.scroll_x;
	uint16_t tile_map_start = 0x1800; // Initial offset

	// Pixel offsets
	uint16_t pixel_data_start = state->lcdc.lcd_control.bg_char_sel ? 0x0 : 0x800;
	uint8_t pixel_y_offset = (sy & 7) * 2;

	uint16_t pixel_temp = 0;

	if(state->lcdc.lcd_control.bg_code_sel)
	{
		tile_map_start += 0x400;
	}

	for(x = 0; x < 160; x++, sx++)
	{
		if(!((sx & 7) && x))
		{
			const uint16_t tile_index = s_sy * 32 + (sx / 8);
			uint8_t tile = state->lcdc.vram[0x0][tile_map_start + tile_index];
			uint16_t s = 15, t;
			uint8_t *mem;

			if(!state->lcdc.lcd_control.bg_char_sel)
			{
				tile -= 0x80;
			}

			// Position in memory
			mem = state->lcdc.vram[0x0] + pixel_data_start + (tile * 16) + pixel_y_offset;

			// Interleave bits and reverse
			t = pixel_temp = interleave8(0, *mem, 0, *(mem + 1));

			for(t >>= 1; t; t >>= 1, s--)
			{
				pixel_temp <<= 1;
				pixel_temp |= t & 1;
			}
			pixel_temp <<= s;
		}

		state->lcdc.out[state->lcdc.ly][x] = dmg_palette[pixel_temp & 0x3];
		pixel_temp >>= 2;
	}
}

static inline void dmg_oam_render(emu_state *restrict state)
{
	uint8_t curr_tile = 0;
	uint8_t pixel_y_offset;
	uint32_t *row = state->lcdc.out[state->lcdc.ly];
	uint8_t y_len = (state->lcdc.lcd_control.obj_block_size) ? 16 : 8;
	uint8_t tx, tx_dest;

	if(!state->lcdc.lcd_control.obj)
	{
		return;
	}

	for(; curr_tile < 40; curr_tile++)
	{
		uint8_t *mem;
		uint16_t pixel_temp;
		uint16_t s = 15, t;
		oam obj = state->lcdc.oam_store[curr_tile];
		uint8_t tile = obj.chr;
		uint8_t actual_x = obj.x;

		pixel_y_offset = state->lcdc.ly - obj.y;

		if(pixel_y_offset > (y_len))
		{
			// out of display
			continue;
		}

		// Position in memory
		mem = state->lcdc.vram[0x0] + (tile * (y_len * 2)) + (pixel_y_offset * 2);

		// Interleave bits and reverse
		t = pixel_temp = interleave8(0, *mem, 0, *(mem + 1));

		for(t >>= 1; t; t >>= 1, s--)
		{
			pixel_temp <<= 1;
			pixel_temp |= t & 1;
		}
		pixel_temp <<= s;

		if(obj.hflip)
		{
			tx = 8;
			tx_dest = 0;
		}
		else
		{
			tx = 0;
			tx_dest = 8;
		}

		for (; tx != tx_dest; pixel_temp >>= 2)
		{
			if(obj.hflip) { tx--; }
			else { tx++; }

			//if((pixel_temp & 0x03) == 0) continue; // invisible.
			if(actual_x + tx > 160) continue; // off screen
			//if(!obj.priority && row[actual_x] != dmg_palette[0]) continue; // hidden
			row[actual_x + tx] = dmg_palette[pixel_temp & 0x3] + 100;
		}
	}
}

void lcdc_tick(emu_state *restrict state)
{
	if(unlikely(state->stop) ||
	   unlikely(!state->lcdc.lcd_control.enable))
	{
		return;
	}

	state->lcdc.curr_clk++;

	switch(state->lcdc.stat.mode_flag)
	{
	case 2:
		// first mode - reading OAM for h scan line
		if(state->lcdc.curr_clk >= 80)
		{
			state->lcdc.curr_clk = 0;
			state->lcdc.stat.mode_flag = 3;
		}
		break;
	case 3:
		// second mode - reading VRAM for h scan line
		if(state->lcdc.curr_clk >= 172)
		{
			state->lcdc.curr_clk = 0;
			state->lcdc.stat.mode_flag = 0;
		}
		break;
	case 0:
		// third mode - h-blank
		if(state->lcdc.curr_clk >= 204)
		{
			switch(state->system)
			{
			case SYSTEM_DMG:
			case SYSTEM_MGB:
			case SYSTEM_MGL:
			case SYSTEM_SGB:
			case SYSTEM_SGB2:
				dmg_bg_render(state);
				dmg_oam_render(state);
				break;
			case SYSTEM_CGB:
			default:
				fatal(state, "No CGB support yet, sorry!");
				break;
			}

			state->lcdc.curr_clk = 0;
			if((++state->lcdc.ly) == 144)
			{
				// going to v-blank
				state->lcdc.stat.mode_flag = 1;
			}
			else
			{
				// start another scan line
				state->lcdc.stat.mode_flag = 2;
			}
		}
		break;
	case 1:
		// v-blank
		if(state->lcdc.ly == 144 &&
		   state->lcdc.curr_clk == 1)
		{
			// Fire the vblank interrupt
			signal_interrupt(state, INT_VBLANK);

			// Blit
			BLIT_CANVAS(state);
		}

		if(state->lcdc.curr_clk % 456 == 0)
		{
			state->lcdc.ly++;
		};

		if(state->lcdc.ly == 153)
		{
			state->lcdc.curr_clk = 0;
			state->lcdc.ly = 0;
			state->lcdc.stat.mode_flag = 2;
		}

		break;
	default:
		fatal(state, "somehow wound up in an unknown impossible video mode");
	}

	if(state->lcdc.ly == state->lcdc.lyc)
	{
		state->lcdc.stat.lyc_state = true;
		if(state->lcdc.stat.lyc)
		{
			signal_interrupt(state, INT_LCD_STAT);
		}
	}
	else
	{
		state->lcdc.stat.lyc_state = false;
	}
}

inline uint8_t lcdc_read(emu_state *restrict state, uint16_t reg)
{
	error(state, "lcdc: unknown register %04X (R)", reg);
	return 0xFF;
}

inline uint8_t vram_read(emu_state *restrict state, uint16_t reg)
{
	uint8_t curr_mode = state->lcdc.stat.mode_flag;
	uint8_t bank = state->lcdc.vram_bank;
	if(curr_mode > 2)
	{
		// Game freak write shitty code and write to VRAM anyway.
		// Pokémon RGB break if we fatal here.
		warning(state, "read from VRAM while not in h/v-blank");
		return 0xFF;
	}

	return state->lcdc.vram[bank][reg - 0x8000];
}

inline uint8_t lcdc_control_read(emu_state *restrict state, uint16_t reg UNUSED)
{
	return state->lcdc.lcd_control.reg;
}

inline uint8_t lcdc_stat_read(emu_state *restrict state, uint16_t reg UNUSED)
{
	return state->lcdc.stat.reg;
}

inline uint8_t lcdc_scroll_read(emu_state *restrict state, uint16_t reg)
{
	if(reg == 0xFF42)
	{
		return state->lcdc.scroll_y;
	}
	else if(reg == 0xFF43)
	{
		return state->lcdc.scroll_x;
	}
	else
	{
		fatal(state, "BUG: attempt to read scroll stuff from non-scroll reg");
		return 0xFF;
	}
}

inline uint8_t lcdc_ly_read(emu_state *restrict state, uint16_t reg UNUSED)
{
	return state->lcdc.ly;
}

inline uint8_t lcdc_lyc_read(emu_state *restrict state, uint16_t reg UNUSED)
{
	return state->lcdc.lyc;
}

inline uint8_t lcdc_bgp_read(emu_state *restrict state, uint16_t reg UNUSED)
{
	return state->lcdc.bg_pal.reg;
}

inline uint8_t lcdc_objp_read(emu_state *restrict state, uint16_t reg)
{
	return state->lcdc.obj_pal[reg - 0xFF48].reg;
}

inline uint8_t lcdc_window_read(emu_state *restrict state, uint16_t reg)
{
	if(reg == 0xFF4A)
	{
		return state->lcdc.window_y;
	}
	else if(reg == 0xFF4B)
	{
		return state->lcdc.window_x;
	}
	else
	{
		fatal(state, "BUG: Attempt to read window stuff from non-window reg");
		return 0xFF;
	}
}

inline uint8_t bg_pal_ind_read(emu_state *restrict state, uint16_t reg)
{
	if(state->system != SYSTEM_CGB)
	{
		return no_hardware(state, reg);
	}

	// TODO
	return state->memory[reg];
}

inline uint8_t bg_pal_data_read(emu_state *restrict state, uint16_t reg)
{
	if(state->system != SYSTEM_CGB)
	{
		return no_hardware(state, reg);
	}

	// TODO
	return state->memory[reg];
}

inline uint8_t sprite_pal_ind_read(emu_state *restrict state, uint16_t reg)
{
	if(state->system != SYSTEM_CGB)
	{
		return no_hardware(state, reg);
	}

	// TODO
	return state->memory[reg];
}

inline uint8_t sprite_pal_data_read(emu_state *restrict state, uint16_t reg)
{
	if(state->system != SYSTEM_CGB)
	{
		return no_hardware(state, reg);
	}

	// TODO
	return state->memory[reg];
}

void dump_lcdc_state(emu_state *restrict state)
{
	debug(state, "---LCDC---");
	debug(state, "CTRL: %02X (%s %s %s %s %s %s %s %s)",
	      state->lcdc.lcd_control.reg,
	      (state->lcdc.lcd_control.dmg_bg) ? "BG" : "bg",
	      (state->lcdc.lcd_control.obj) ? "OBJ" : "obj",
	      (state->lcdc.lcd_control.obj_block_size) ? "8x16" : "8x8",
	      (state->lcdc.lcd_control.bg_code_sel) ? "9C" : "98",
	      (state->lcdc.lcd_control.bg_char_sel) ? "sign" : "SIGN",
	      (state->lcdc.lcd_control.win) ? "WIN" : "win",
	      (state->lcdc.lcd_control.win_code_sel) ? "9C" : "98",
	      (state->lcdc.lcd_control.enable) ? "E" : "e"
	);
	debug(state, "STAT: %02X (MODE=%d)",
	      state->lcdc.stat.reg, state->lcdc.stat.mode_flag);
	debug(state, "ICLK: %02X", state->lcdc.curr_clk);
	debug(state, "LY  : %02X", state->lcdc.ly);
}

inline void lcdc_write(emu_state *restrict state, uint16_t reg, uint8_t data UNUSED)
{
	error(state, "lcdc: unknown register %04X (W)", reg);
}

inline void vram_write(emu_state *restrict state, uint16_t reg, uint8_t data)
{
	uint8_t curr_mode = state->lcdc.stat.mode_flag;
	uint8_t bank = state->lcdc.vram_bank;
	if(curr_mode > 2)
	{
		// Game freak write shitty code and write to VRAM anyway.
		// Pokémon RGB break if we fatal here.
		// Pass it through anyway otherwise things look "wrong"
		warning(state, "write to VRAM while not in h/v-blank");
	}

	state->lcdc.vram[bank][reg - 0x8000] = data;
}

inline void lcdc_control_write(emu_state *restrict state, uint16_t reg UNUSED, uint8_t data)
{
	state->lcdc.lcd_control.reg = data;
}

inline void lcdc_stat_write(emu_state *restrict state, uint16_t reg UNUSED, uint8_t data)
{
	state->lcdc.stat.lyc = ((data & 0x60) == 0x60);
}

inline void lcdc_scroll_write(emu_state *restrict state, uint16_t reg, uint8_t data)
{
	if(reg == 0xFF42)
	{
		state->lcdc.scroll_y = data;
	}
	else if(reg == 0xFF43)
	{
		state->lcdc.scroll_x = data;
	}
	else
	{
		fatal(state, "BUG: attempt to write scroll stuff to non-scroll reg");
	}
}

inline void lcdc_ly_write(emu_state *restrict state, uint16_t reg UNUSED, uint8_t data UNUSED)
{
#ifndef NDEBUG
	fatal(state, "write to LY (FF44); you can't just vsync yourself!");
#else
	error(state, "write to LY (FF44) is being ignored");
#endif
}

inline void lcdc_lyc_write(emu_state *restrict state, uint16_t reg UNUSED, uint8_t data)
{
	state->lcdc.lyc = data;
}

inline void lcdc_bgp_write(emu_state *restrict state, uint16_t reg UNUSED, uint8_t data)
{
	state->lcdc.bg_pal.reg = data;
}

inline void lcdc_objp_write(emu_state *restrict state, uint16_t reg, uint8_t data)
{
	state->lcdc.obj_pal[reg - 0xFF48].reg = data;
}

inline void lcdc_window_write(emu_state *restrict state, uint16_t reg, uint8_t data)
{
	if(reg == 0xFF4A)
	{
		state->lcdc.window_y = data;
	}
	else if(reg == 0xFF4B)
	{
		state->lcdc.window_x = data;
	}
	else
	{
		fatal(state, "BUG: Attempt to write window data to non-window register");
	}
}

void bg_pal_ind_write(emu_state *restrict state, uint16_t reg, uint8_t data)
{
	if(state->system != SYSTEM_CGB)
	{
		doofus_write(state, reg, data);
		return;
	}

	// TODO
	state->memory[reg] = data;
}

void bg_pal_data_write(emu_state *restrict state, uint16_t reg, uint8_t data)
{
	if(state->system != SYSTEM_CGB)
	{
		doofus_write(state, reg, data);
		return;
	}

	// TODO
	state->memory[reg] = data;
}

void sprite_pal_ind_write(emu_state *restrict state, uint16_t reg, uint8_t data)
{
	if(state->system != SYSTEM_CGB)
	{
		doofus_write(state, reg, data);
		return;
	}

	// TODO
	state->memory[reg] = data;
}

void sprite_pal_data_write(emu_state *restrict state, uint16_t reg, uint8_t data)
{
	if(state->system != SYSTEM_CGB)
	{
		doofus_write(state, reg, data);
		return;
	}

	// TODO
	state->memory[reg] = data;
}

void magical_mystery_cure(void)
{
	lcdc_read(NULL, 0);
	lcdc_control_read(NULL, 0);
	lcdc_stat_read(NULL, 0);
	lcdc_scroll_read(NULL, 0);
	lcdc_ly_read(NULL, 0);
	lcdc_lyc_read(NULL, 0);
	lcdc_bgp_read(NULL, 0);
	lcdc_objp_read(NULL, 0);
	lcdc_window_read(NULL, 0);
	bg_pal_ind_read(NULL, 0);
	bg_pal_data_read(NULL, 0);
	sprite_pal_ind_read(NULL, 0);
	sprite_pal_data_read(NULL, 0);
	vram_read(NULL, 0);
	lcdc_write(NULL, 0, 0);
	lcdc_control_write(NULL, 0, 0);
	lcdc_stat_write(NULL, 0, 0);
	lcdc_scroll_write(NULL, 0, 0);
	lcdc_ly_write(NULL, 0, 0);
	lcdc_lyc_write(NULL, 0, 0);
	lcdc_bgp_write(NULL, 0, 0);
	lcdc_objp_write(NULL, 0, 0);
	lcdc_window_write(NULL, 0, 0);
	vram_write(NULL, 0, 0);
}
