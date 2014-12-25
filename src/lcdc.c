#include "config.h"	// macros

#include "print.h"	// fatal
#include "ctl_unit.h"	// signal_interrupt
#include "util.h"	// likely/unlikely
#include "sgherm.h"	// emu_state
#include "util_bitops.h"// bitops

#include <assert.h>
#include <string.h>	// memset


static const uint32_t dmg_palette[4] =
{
	0x00FFFFFF,
	0x00AAAAAA,
	0x00555555,
	0x00000000,
};

void init_lcdc(emu_state *restrict state)
{
	state->lcdc.lcd_control.enable = 1;
	state->lcdc.lcd_control.bg_char_sel = 1;
	state->lcdc.lcd_control.dmg_bg = 1;

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
		uint8_t pixel;

		if(!(x && (sx & 7)))
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

		pixel = (state->lcdc.bg_pal >> ((pixel_temp & 3) * 2)) & 0x3;
		state->lcdc.out[state->lcdc.ly][x] = dmg_palette[pixel];
		pixel_temp >>= 2;
	}
}

static inline void dmg_window_render(emu_state *restrict state)
{
	const int16_t y = state->lcdc.ly, wy = y - state->lcdc.window_y;
	uint16_t x = 0;
	int16_t wx = state->lcdc.window_x - 7;
	uint16_t tile_map_start = state->lcdc.lcd_control.win_code_sel ? 0x1c00 : 0x1800;

	// Pixel offsets
	uint8_t pixel_y_offset = (wy & 7) * 2;

	// Yes, windows use this.
	uint16_t pixel_data_start = state->lcdc.lcd_control.bg_char_sel ? 0x0 : 0x800;

	uint16_t pixel_temp = 0;

	if(wx > 159 || wy > 143 || wy < 0)
	{
		// Off-screen
		return;
	}

	for(; wx < 159; x++, wx++)
	{
		uint8_t pixel;

		if((wx & 7) == 0 || x == 0)
		{
			const uint16_t tile_index = (wy / 8) * 32 + (x / 8);
			uint8_t tile = state->lcdc.vram[0x0][tile_map_start + tile_index];
			uint8_t *mem;
			uint16_t s = 15, t;

			if(!state->lcdc.lcd_control.bg_char_sel)
			{
				tile -= 0x80;
			}

			mem = state->lcdc.vram[0x0] + (tile * 16) + pixel_data_start + pixel_y_offset;

			// Interleave bits and reverse
			t = pixel_temp = interleave8(0, *mem, 0, *(mem + 1));

			for(t >>= 1; t; t >>= 1, s--)
			{
				pixel_temp <<= 1;
				pixel_temp |= t & 1;
			}
			pixel_temp <<= s;
		}

		if(wx < 0)
		{
			continue;
		}

		pixel = (state->lcdc.bg_pal >> ((pixel_temp & 3) * 2)) & 0x3;
		state->lcdc.out[y][wx] = dmg_palette[pixel];
		pixel_temp >>= 2;
	}
}

static inline void dmg_oam_render(emu_state *restrict state)
{
	int curr_tile;
	uint16_t pixel_y_offset;
	uint32_t *row = state->lcdc.out[state->lcdc.ly];
	uint8_t y_len = (state->lcdc.lcd_control.obj_block_size) ? 16 : 8;
	uint16_t tx;

	if(!state->lcdc.lcd_control.obj)
	{
		return;
	}

	for(curr_tile = 39; curr_tile >= 0; curr_tile -= 1)
	{
		oam *obj = (oam *)(state->lcdc.oam_ram + (4 * curr_tile));
		uint8_t tile = obj->chr;
		const int16_t obj_x = obj->x - 8, obj_y = obj->y - 16;

		uint8_t *mem;
		uint16_t pixel_temp;
		uint16_t s = 15, t;

		// Adjusted for offsets
		if(!(obj->x && obj->y && obj->x < 168 && obj->y < 160))
		{
			// Off-screen
			continue;
		}

		pixel_y_offset = state->lcdc.ly - obj_y;
		if(pixel_y_offset > (y_len - 1))
		{
			// out of display
			continue;
		}

		if(obj->vflip)
		{
			pixel_y_offset = y_len - 1 - pixel_y_offset;
		}

		if(pixel_y_offset > 7)
		{
			// Bottom tile
			pixel_y_offset = (pixel_y_offset - 8) * 2;
			mem = state->lcdc.vram[0x0] + ((tile | 0x01) * 16) + pixel_y_offset;
		}
		else
		{
			if (y_len == 16) tile &= 0xFE;
			// Top tile
			pixel_y_offset *= 2;
			mem = state->lcdc.vram[0x0] + (tile * 16) + pixel_y_offset;
		}

		t = pixel_temp = interleave8(0, *mem, 0, *(mem + 1));

		// Interleave bits and reverse

		for(t >>= 1; t; t >>= 1, s--)
		{
			pixel_temp <<= 1;
			pixel_temp |= t & 1;
		}
		pixel_temp <<= s;

		if(obj->hflip)
		{
			// XXX this is bogus
			pixel_temp = rotl_16(pixel_temp, 2);
		}

		for(tx = 0; tx < 8; tx++)
		{
			if((pixel_temp & 0x03) && ((obj_x + tx) <= 159) &&
				((obj_x + tx) >= 0) && (!obj->priority ||
				(obj->priority && row[obj_x + tx] ==
				 dmg_palette[0])))
			{
				const uint8_t pal = state->lcdc.obj_pal[-obj->pal_dmg];
				const uint8_t pixel = (pal >> ((pixel_temp & 3) * 2)) & 0x3;
				row[obj_x + tx] = dmg_palette[pixel];
			}

			if(obj->hflip)
			{
				pixel_temp = rotl_16(pixel_temp, 2);
			}
			else
			{
				pixel_temp >>= 2;
			}
		}
	}
}

static inline void lcdc_mode_change(emu_state *restrict state, uint8_t mode)
{
#ifndef NDEBUG
	assert(mode < 4);
#endif

	state->lcdc.curr_clk = 0;
	state->lcdc.stat.mode_flag = mode;
	if(mode < 3 && state->lcdc.stat.reg & (1 << (mode + 3)))
	{
		signal_interrupt(state, INT_LCD_STAT);
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
			lcdc_mode_change(state, 3);
		}
		break;
	case 3:
		// second mode - reading VRAM for h scan line
		if(state->lcdc.curr_clk >= 172)
		{
			lcdc_mode_change(state, 0);
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
				if(state->lcdc.lcd_control.dmg_bg)
				{
					dmg_bg_render(state);
				}
				else
				{
					memset(state->lcdc.out, dmg_palette[0],
							sizeof(state->lcdc.out));
				}

				if(state->lcdc.lcd_control.win)
				{
					dmg_window_render(state);
				}

				if(state->lcdc.lcd_control.obj)
				{
					dmg_oam_render(state);
				}
				break;
			case SYSTEM_CGB:
			default:
				fatal(state, "No CGB support yet, sorry!");
				break;
			}

			if((++state->lcdc.ly) == 144)
			{
				// going to v-blank
				lcdc_mode_change(state, 1);
			}
			else
			{
				// start another scan line
				lcdc_mode_change(state, 2);
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
			state->lcdc.ly = 0;
			lcdc_mode_change(state, 2);
		}

		break;
	default:
		fatal(state, "somehow wound up in an unknown impossible video mode");
	}

	if(state->lcdc.ly == state->lcdc.lyc)
	{
		state->lcdc.stat.lyc_state = 1;
		if(state->lcdc.stat.lyc)
		{
			signal_interrupt(state, INT_LCD_STAT);
		}
	}
	else
	{
		state->lcdc.stat.lyc_state = 0;
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
		// Every freaking game seems to do this.
		//warning(state, "read from VRAM while not in h/v-blank");
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
	return state->lcdc.bg_pal;
}

inline uint8_t lcdc_objp_read(emu_state *restrict state, uint16_t reg)
{
	return state->lcdc.obj_pal[reg - 0xFF48];
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
	return 0xFF;
}

inline uint8_t bg_pal_data_read(emu_state *restrict state, uint16_t reg)
{
	if(state->system != SYSTEM_CGB)
	{
		return no_hardware(state, reg);
	}

	// TODO
	return 0xFF;
}

inline uint8_t sprite_pal_ind_read(emu_state *restrict state, uint16_t reg)
{
	if(state->system != SYSTEM_CGB)
	{
		return no_hardware(state, reg);
	}

	// TODO
	return 0xFF;
}

inline uint8_t sprite_pal_data_read(emu_state *restrict state, uint16_t reg)
{
	if(state->system != SYSTEM_CGB)
	{
		return no_hardware(state, reg);
	}

	// TODO
	return 0xFF;
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
		// Every freaking game seems to do this
		//warning(state, "write to VRAM while not in h/v-blank");
	}

	state->lcdc.vram[bank][reg - 0x8000] = data;
}

inline void lcdc_control_write(emu_state *restrict state, uint16_t reg UNUSED, uint8_t data)
{
	state->lcdc.lcd_control.reg = data;
}

inline void lcdc_stat_write(emu_state *restrict state, uint16_t reg UNUSED, uint8_t data)
{
	/* don't overwrite mode bits */
	state->lcdc.stat.reg = (data & 0x78) | state->lcdc.stat.mode_flag;
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
	state->lcdc.bg_pal = data;
}

inline void lcdc_objp_write(emu_state *restrict state, uint16_t reg, uint8_t data)
{
	state->lcdc.obj_pal[reg - 0xFF48] = data;
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
}

void bg_pal_data_write(emu_state *restrict state, uint16_t reg, uint8_t data)
{
	if(state->system != SYSTEM_CGB)
	{
		doofus_write(state, reg, data);
		return;
	}

	// TODO
}

void sprite_pal_ind_write(emu_state *restrict state, uint16_t reg, uint8_t data)
{
	if(state->system != SYSTEM_CGB)
	{
		doofus_write(state, reg, data);
		return;
	}

	// TODO
}

void sprite_pal_data_write(emu_state *restrict state, uint16_t reg, uint8_t data)
{
	if(state->system != SYSTEM_CGB)
	{
		doofus_write(state, reg, data);
		return;
	}

	// TODO
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
