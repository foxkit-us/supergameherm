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
	int i;

	// Enable LCD + BG char sel + BG
	state->lcdc.lcd_control = 0x91;
	state->lcdc.next_clk = 0;

	// Initialise to mode 2
	state->lcdc.stat = 2;

	// Palette init
	state->lcdc.bg_pal = 0xFC;
	state->lcdc.obj_pal[0] = state->lcdc.obj_pal[1] = 0xFF;

	// CGB palette init
	for(i = 0; i < 64; i++)
	{
		state->lcdc.bcpd[i] = 0xFF;
		state->lcdc.ocpd[i] = 0xFF;
	}

	for(i = 0; i < 32; i++)
	{
		state->lcdc.bcpal[i] = 0x00FFFFFF;
		state->lcdc.ocpal[i] = 0x00FFFFFF;
	}

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
	uint16_t pixel_data_start = LCDC_BG_CHAR_SEL(state) ? 0x0 : 0x800;
	uint8_t pixel_y_offset = (sy & 7) * 2;

	uint16_t pixel_temp = 0;

	if(LCDC_BG_CODE_SEL(state))
	{
		tile_map_start += 0x400;
	}

	for(x = 0; x < 160; x++, sx++)
	{
		uint8_t pixel;

		if(x == 0 || (sx & 7) == 0)
		{
			const uint16_t tile_index = s_sy * 32 + (sx / 8);
			uint8_t tile = state->lcdc.vram[0x0][tile_map_start + tile_index];
			uint8_t *mem;

			if(!LCDC_BG_CHAR_SEL(state))
			{
				tile -= 0x80;
			}

			// Position in memory
			mem = state->lcdc.vram[0x0] + pixel_data_start + (tile * 16) + pixel_y_offset;

			// Interleave bits and reverse
			pixel_temp = interleave8(0, *mem, 0, *(mem + 1)) & 0xFFFF;

			// XXX kinda bogus but needed to make it look right
			pixel_temp = rotl_16(pixel_temp, 2);

			if(x == 0)
			{
				// Compensate for off-screen pixels
				pixel_temp = rotl_16(pixel_temp, 2 * (sx & 7));
			}
		}

		pixel = (state->lcdc.bg_pal >> ((pixel_temp & 3) * 2)) & 0x3;
		state->lcdc.out[state->lcdc.ly][x] = dmg_palette[pixel];
		pixel_temp = rotl_16(pixel_temp, 2);
	}
}

static inline void cgb_bg_render(emu_state *restrict state)
{
	// Compute positions in the "virtual" map of tiles
	const uint8_t sy = state->lcdc.ly + state->lcdc.scroll_y, s_sy = sy / 8;
	uint8_t x, sx = state->lcdc.scroll_x;
	uint16_t tile_map_start = 0x1800; // Initial offset

	// Pixel offsets
	uint16_t pixel_data_start = LCDC_BG_CHAR_SEL(state) ? 0x0 : 0x800;
	uint8_t pixel_y_offset = (sy & 7) * 2;

	uint16_t pixel_temp = 0;
	uint32_t *palette = state->lcdc.bcpal;

	if(LCDC_BG_CODE_SEL(state))
	{
		tile_map_start += 0x400;
	}

	for(x = 0; x < 160; x++, sx++)
	{
		//uint8_t pixel;

		if(x == 0 || (sx & 7) == 0)
		{
			const uint16_t tile_index = s_sy * 32 + (sx / 8);
			uint8_t tile = state->lcdc.vram[0x0][tile_map_start + tile_index];
			uint8_t attr = state->lcdc.vram[0x1][tile_map_start + tile_index];
			uint8_t *mem;

			// Get attribute information
			int tbank = ((attr&0x08) != 0 ? 1 : 0);
			palette = state->lcdc.bcpal + ((attr&7)<<2);

			// TODO: hflip/vflip/prio

			if(!LCDC_BG_CHAR_SEL(state))
			{
				tile -= 0x80;
			}

			// Position in memory
			mem = state->lcdc.vram[tbank] + pixel_data_start + (tile * 16) + pixel_y_offset;

			// Interleave bits and reverse
			pixel_temp = interleave8(0, *mem, 0, *(mem + 1)) & 0xFFFF;

			// XXX kinda bogus but needed to make it look right
			pixel_temp = rotl_16(pixel_temp, 2);

			if(x == 0)
			{
				// Compensate for off-screen pixels
				pixel_temp = rotl_16(pixel_temp, 2 * (sx & 7));
			}
		}

		state->lcdc.out[state->lcdc.ly][x] = palette[pixel_temp & 3];
		pixel_temp = rotl_16(pixel_temp, 2);
	}
}

static inline void dmg_window_render(emu_state *restrict state)
{
	const int16_t y = state->lcdc.ly, wy = y - state->lcdc.window_y;
	uint16_t x = 0;
	int16_t wx = state->lcdc.window_x - 7;
	uint16_t tile_map_start = LCDC_WIN_CODE_SEL(state) ? 0x1c00 : 0x1800;

	// Pixel offsets
	uint8_t pixel_y_offset = (wy & 7) * 2;

	// Yes, windows use this.
	uint16_t pixel_data_start = LCDC_BG_CHAR_SEL(state) ? 0x0 : 0x800;

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

			if(!LCDC_BG_CHAR_SEL(state))
			{
				tile -= 0x80;
			}

			mem = state->lcdc.vram[0x0] + (tile * 16) + pixel_data_start + pixel_y_offset;

			// Interleave bits and reverse
			pixel_temp = interleave8(0, *mem, 0, *(mem + 1)) & 0xFFFF;

			// XXX
			pixel_temp = rotl_16(pixel_temp, 2);
		}

		if(wx < 0)
		{
			continue;
		}

		pixel = (state->lcdc.bg_pal >> ((pixel_temp & 3) * 2)) & 0x3;
		state->lcdc.out[y][wx] = dmg_palette[pixel];
		pixel_temp = rotl_16(pixel_temp, 2);
	}
}

static inline void cgb_window_render(emu_state *restrict state)
{
	const int16_t y = state->lcdc.ly, wy = y - state->lcdc.window_y;
	uint16_t x = 0;
	int16_t wx = state->lcdc.window_x - 7;
	uint16_t tile_map_start = LCDC_WIN_CODE_SEL(state) ? 0x1c00 : 0x1800;

	// Pixel offsets
	uint8_t pixel_y_offset = (wy & 7) * 2;

	// Yes, windows use this.
	uint16_t pixel_data_start = LCDC_BG_CHAR_SEL(state) ? 0x0 : 0x800;

	uint16_t pixel_temp = 0;
	uint32_t *palette = state->lcdc.bcpal;

	if(wx > 159 || wy > 143 || wy < 0)
	{
		// Off-screen
		return;
	}

	for(; wx < 159; x++, wx++)
	{
		//uint8_t pixel;

		if((wx & 7) == 0 || x == 0)
		{
			const uint16_t tile_index = (wy / 8) * 32 + (x / 8);
			uint8_t tile = state->lcdc.vram[0x0][tile_map_start + tile_index];
			uint8_t attr = state->lcdc.vram[0x1][tile_map_start + tile_index];
			uint8_t *mem;

			// Get attribute information
			int tbank = ((attr&0x08) != 0 ? 1 : 0);
			palette = state->lcdc.bcpal + ((attr&7)<<2);

			// TODO: hflip/vflip/prio

			if(!LCDC_BG_CHAR_SEL(state))
			{
				tile -= 0x80;
			}

			mem = state->lcdc.vram[tbank] + (tile * 16) + pixel_data_start + pixel_y_offset;

			// Interleave bits and reverse
			pixel_temp = interleave8(0, *mem, 0, *(mem + 1)) & 0xFFFF;

			// XXX
			pixel_temp = rotl_16(pixel_temp, 2);
		}

		if(wx < 0)
		{
			continue;
		}

		state->lcdc.out[y][wx] = palette[pixel_temp & 3];
		pixel_temp = rotl_16(pixel_temp, 2);
	}
}

static inline void copy_oam(emu_state *state, uint8_t tile, oam *obj)
{
	uint8_t *mem = state->lcdc.oam_ram + (4 * tile);

	obj->y         = mem[0];
	obj->x         = mem[1];
	obj->chr       = mem[2];

	obj->pal_cgb   = (mem[3] & 0x07);
	obj->char_bank = (mem[3] & 0x08) >> 3;
	obj->pal_dmg   = (mem[3] & 0x10) >> 4;
	obj->hflip     = (mem[3] & 0x20) >> 5;
	obj->vflip     = (mem[3] & 0x40) >> 6;
	obj->priority  = (mem[3] & 0x80) >> 7;
}

static inline void dmg_oam_render(emu_state *restrict state)
{
	int curr_tile;
	uint16_t pixel_y_offset;
	uint32_t *row = state->lcdc.out[state->lcdc.ly];
	uint8_t y_len = (LCDC_OBJ_SIZE(state)) ? 16 : 8;
	uint16_t tx;

	if(!LCDC_OBJ(state))
	{
		return;
	}

	for(curr_tile = 39; curr_tile >= 0; curr_tile -= 1)
	{
		oam obj;
		uint8_t tile;
		int16_t obj_x, obj_y;

		uint8_t *mem;
		uint16_t pixel_temp;

		copy_oam(state, curr_tile, &obj);
		tile = obj.chr;
		obj_x = obj.x - 8;
		obj_y = obj.y - 16;

		// Adjusted for offsets
		if(!(obj.x && obj.y && obj.x < 168 && obj.y < 160))
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

		if(obj.vflip)
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

		// Interleave bits and reverse
		pixel_temp = interleave8(0, *mem, 0, *(mem + 1)) & 0xFFFF;

		if(!(obj.hflip))
		{
			// XXX
			pixel_temp = rotl_16(pixel_temp, 2);
		}

		for(tx = 0; tx < 8; tx++)
		{
			if((pixel_temp & 0x03) && ((obj_x + tx) <= 159) &&
				((obj_x + tx) >= 0) && (!obj.priority ||
				(obj.priority && row[obj_x + tx] ==
				 dmg_palette[0])))
			{
				const uint8_t pal = state->lcdc.obj_pal[obj.pal_dmg];
				const uint8_t pixel = (pal >> ((pixel_temp & 3) * 2)) & 0x3;
				row[obj_x + tx] = dmg_palette[pixel];
			}

			if(obj.hflip)
			{
				pixel_temp >>= 2;
			}
			else
			{
				pixel_temp = rotl_16(pixel_temp, 2);
			}
		}
	}
}

static inline void cgb_oam_render(emu_state *restrict state)
{
	int curr_tile;
	uint16_t pixel_y_offset;
	uint32_t *row = state->lcdc.out[state->lcdc.ly];
	uint8_t y_len = (LCDC_OBJ_SIZE(state)) ? 16 : 8;
	uint16_t tx;

	if(!LCDC_OBJ(state))
	{
		return;
	}

	for(curr_tile = 39; curr_tile >= 0; curr_tile -= 1)
	{
		oam obj;
		uint8_t tile;
		int16_t obj_x, obj_y;

		uint8_t *mem;
		uint32_t *palette = state->lcdc.ocpal;
		uint16_t pixel_temp;
		int tbank;

		copy_oam(state, curr_tile, &obj);
		tile = obj.chr;
		obj_x = obj.x - 8;
		obj_y = obj.y - 16;

		// Adjusted for offsets
		if(!(obj.x && obj.y && obj.x < 168 && obj.y < 160))
		{
			// Off-screen
			continue;
		}

		// Get attribute information
		palette = state->lcdc.ocpal + (obj.pal_cgb<<2);
		tbank = obj.char_bank;

		pixel_y_offset = state->lcdc.ly - obj_y;
		if(pixel_y_offset > (y_len - 1))
		{
			// out of display
			continue;
		}

		if(obj.vflip)
		{
			pixel_y_offset = y_len - 1 - pixel_y_offset;
		}

		if(pixel_y_offset > 7)
		{
			// Bottom tile
			pixel_y_offset = (pixel_y_offset - 8) * 2;
			mem = state->lcdc.vram[tbank] + ((tile | 0x01) * 16) + pixel_y_offset;
		}
		else
		{
			if (y_len == 16) tile &= 0xFE;
			// Top tile
			pixel_y_offset *= 2;
			mem = state->lcdc.vram[tbank] + (tile * 16) + pixel_y_offset;
		}

		// Interleave bits and reverse
		pixel_temp = interleave8(0, *mem, 0, *(mem + 1)) & 0xFFFF;

		if(!(obj.hflip))
		{
			// XXX
			pixel_temp = rotl_16(pixel_temp, 2);
		}

		for(tx = 0; tx < 8; tx++)
		{
			// TODO: proper priority
			if((pixel_temp & 0x03) && ((obj_x + tx) <= 159) &&
				((obj_x + tx) >= 0) && (!obj.priority ||
				(obj.priority && row[obj_x + tx] ==
				 dmg_palette[0])))
			{
				row[obj_x + tx] = palette[pixel_temp & 0x3];
			}

			if(obj.hflip)
			{
				pixel_temp >>= 2;
			}
			else
			{
				pixel_temp = rotl_16(pixel_temp, 2);
			}
		}
	}
}

void lcdc_mode_change(emu_state *restrict state, uint8_t mode)
{
	assert(mode < 4);

	state->lcdc.stat = (state->lcdc.stat & ~0x3) | mode;
	switch(mode)
	{
	case 0:
		if(LCDC_STAT_MODE0(state))
		{
			signal_interrupt(state, INT_LCD_STAT);
		}
		break;
	case 1:
		if(LCDC_STAT_MODE1(state) || LCDC_STAT_MODE2(state))
		{
			signal_interrupt(state, INT_LCD_STAT);
		}
		break;
	case 2:
		if(LCDC_STAT_MODE2(state))
		{
			signal_interrupt(state, INT_LCD_STAT);
		}
		break;
	}
}

static inline void render_scanline(emu_state *restrict state)
{
	switch(state->system)
	{
		case SYSTEM_DMG:
		case SYSTEM_MGB:
		case SYSTEM_MGL:
		case SYSTEM_SGB:
		case SYSTEM_SGB2:
			if(LCDC_DMG_BG(state))
			{
				dmg_bg_render(state);
			}
			else
			{
				memset(state->lcdc.out, dmg_palette[0],
				       sizeof(state->lcdc.out));
			}

			if(LCDC_WIN(state))
			{
				dmg_window_render(state);
			}

			if(LCDC_OBJ(state))
			{
				dmg_oam_render(state);
			}
			break;

		case SYSTEM_CGB:
			// TODO: get the actual info for what flag affects what
			if(LCDC_DMG_BG(state))
			{
				cgb_bg_render(state);
			}
			else
			{
				memset(state->lcdc.out, 0x00FFFFFF,
				       sizeof(state->lcdc.out));
			}

			if(LCDC_WIN(state))
			{
				cgb_window_render(state);
			}

			if(LCDC_OBJ(state))
			{
				cgb_oam_render(state);
			}
			break;

		default:
			fatal(state, "No support for this GB type yet, sorry!");
			break;
	}
}

/*!
 * @brief LCD Mode 02 - Reading OAM for current scan line.
 * @details Mode 02 is the first mode of a scanline.  The LCD controller
 * will read the OAM and determine which sprites need to be loaded during
 * drawing.  The OAM area of memory is neither readable nor writable while
 * this mode is active.  This mode typically lasts for 80 clocks, though
 * it may have some variance.  Unknown at this point what causes the
 * variance so we're leaving it a steady 80 for now.
 */
static inline void lcdc_mode2(emu_state *restrict state)
{
	uint8_t clocks = 167;
	static bool lyc_checked = false;

	if(state->lcdc.curr_clk < 80)
	{
		state->lcdc.next_clk = 80;
		if(!lyc_checked)
		{
			if(state->lcdc.ly == state->lcdc.lyc && state->lcdc.curr_clk == 0)
			{
				// Set LYC flag
				state->lcdc.stat |= 0x4;

				if(LCDC_STAT_LYC(state))
				{
					signal_interrupt(state, INT_LCD_STAT);
				}
			}
			else
			{
				state->lcdc.stat &= ~0x4;
			}
			lyc_checked = true;
		}
	}
	else
	{
		lcdc_mode_change(state, 3);
		lyc_checked = false;
		return;
	}

	/* Calculate the amount of clocks Mode 03 will need. */
	clocks += state->lcdc.scroll_x % 7;

	if(LCDC_WIN(state))
	{
		if(state->lcdc.window_x == 0)
		{
			clocks += 7;
		}
		clocks += 6;
	}
	else
	{
		clocks += 7;
	}

	state->lcdc.curr_m3_clks = clocks;
}

/*!
 * @brief LCD Mode 03 - Reading VRAM for current scan line
 * @details Only in this mode is VRAM inaccessible.  IT IS STILL
 * ACCESSIBLE IN ALL OTHER MODES.
 */
static inline void lcdc_mode3(emu_state *restrict state)
{
	uint16_t needed_clk = 80 + state->lcdc.curr_m3_clks;

	if(state->lcdc.curr_clk == needed_clk)
	{
		render_scanline(state);
		lcdc_mode_change(state, 0);
	}
	else
	{
		state->lcdc.next_clk = needed_clk;
	}
}

/*!
 * @brief LCD Mode 00 - Horizontal Blank simulation
 * @details ...
 */
static inline void lcdc_mode0(emu_state *restrict state)
{
	if(state->system < SYSTEM_CGB && state->lcdc.curr_clk == 452)
	{
		++(state->lcdc.ly);
	}
	else if(state->lcdc.curr_clk == 456)
	{
		if(state->system >= SYSTEM_CGB)
		{
			// NOTE: AGB does this also, if we ever emulate that.
			++(state->lcdc.ly);
		}

		if(state->lcdc.ly == 144)
		{
			// going to v-blank
			state->lcdc.curr_clk = state->lcdc.next_clk = 0;
			lcdc_mode_change(state, 1);
		}
		else
		{
			// start another scan line
			state->lcdc.curr_clk = state->lcdc.next_clk = 0;
			lcdc_mode_change(state, 2);
		}
	}
	else if(unlikely(state->lcdc.initial && state->lcdc.ly == 0 && state->lcdc.curr_clk == 80))
	{
		state->lcdc.initial = false;
		state->lcdc.curr_m3_clks = 174 + (state->lcdc.scroll_x % 7);
		lcdc_mode_change(state, 3);
	}
}

/*!
 * @brief LCD Mode 01 - Vertical Blank / refresh
 * @details ...
 */
static inline void lcdc_mode1(emu_state *restrict state)
{
	if(state->lcdc.ly == 144 && state->lcdc.curr_clk == 1)
	{
		// Fire the vblank interrupt
		signal_interrupt(state, INT_VBLANK);
		state->lcdc.throt_trigger = true;

		// Blit
		BLIT_CANVAS(state);
	}

	if(state->lcdc.curr_clk % 456 == 0)
	{
		if(state->lcdc.ly == 0)
		{
			state->lcdc.curr_clk = 0;
			lcdc_mode_change(state, 2);
		}
		else
		{
			state->lcdc.ly++;
		}
	}

	if(state->lcdc.ly == 153 && state->lcdc.curr_clk >= 56)
	{
		state->lcdc.ly = 0;
	}
}

typedef void (*lcdc_mode_fn)(emu_state *restrict);

static lcdc_mode_fn mode_fns[0x4] = {
	lcdc_mode0, lcdc_mode1, lcdc_mode2, lcdc_mode3
};

void lcdc_tick(emu_state *restrict state, int count)
{
#ifdef DEFENSIVE
	if(unlikely(state->stop) ||
	   unlikely(!LCDC_ENABLE(state)))
	{
		return;
	}
#endif

	if((state->lcdc.curr_clk + count) < state->lcdc.next_clk)
	{
		state->lcdc.curr_clk += count;
		return;
	}

	for(; count > 0; count--)
	{
		state->lcdc.curr_clk++;
		mode_fns[LCDC_STAT_MODE_FLAG(state)](state);
	}
}
