/***************************************************************************
 * Gens: VDP rendering functions.                                          *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2009 by David Korth                                  *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
 ***************************************************************************/

#include "vdp_rend.h"
#include "vdp_io.h"

// Video modes.
#include "vdp_rend_m5.hpp"

// bppMD
#include "emulator/g_main.hpp"

// C includes.
#include <stdint.h>

// libgsft includes.
#include "libgsft/gsft_szprintf.h"

// Full MD palettes.
uint16_t Palette[0x1000];
uint32_t Palette32[0x1000];


/**
 * T_VDP_Update_Palette(): VDP palette update function.
 * @param hs If true, updates highlight/shadow.
 * @param MD_palette MD color palette.
 * @param palette Full color palette.
 */
template<bool hs, typename pixel>
static inline void T_VDP_Update_Palette(pixel *MD_palette, const pixel *palette)
{
	if (VDP_Layers & VDP_LAYER_PALETTE_LOCK)
		return;
	
	// Clear the CRam flag, since the palette is being updated.
	VDP_Flags.CRam = 0;
	
	// Color mask. Depends on VDP register 0, bit 2 (Palette Select).
	// If set, allows full MD palette.
	// If clear, only allows the LSB of each color component.
	const uint16_t color_mask = (VDP_Reg.m5.Set1 & 0x04) ? 0x0EEE : 0x0222;
	
	// Update all 64 colors.
	for (int i = 62; i >= 0; i -= 2)
	{
		uint16_t color1_raw = CRam.u16[i] & color_mask;
		uint16_t color2_raw = CRam.u16[i + 1] & color_mask;
		
		// Get the palette color.
		pixel color1 = palette[color1_raw];
		pixel color2 = palette[color2_raw];
		
		// Set the new color.
		MD_palette[i]     = color1;
		MD_palette[i + 1] = color2;
		
		if (hs)
		{
			// Update the highlight and shadow colors.
			// References:
			// - http://www.tehskeen.com/forums/showpost.php?p=71308&postcount=1077
			// - http://forums.sonicretro.org/index.php?showtopic=17905
			
			// Normal color. (xxx0)
			MD_palette[i + 192]	= color1;
			MD_palette[i + 1 + 192]	= color2;
			
			color1_raw >>= 1;
			color2_raw >>= 1;
			
			// Shadow color. (0xxx)
			MD_palette[i + 64]	= palette[color1_raw];
			MD_palette[i + 1 + 64]	= palette[color2_raw];
			
			// Highlight color. (1xxx)
			MD_palette[i + 128]	= palette[0x888 | color1_raw];
			MD_palette[i + 1 + 128]	= palette[0x888 | color2_raw];
		}
	}
	
	// Update the background color.
	unsigned int BG_Color = (VDP_Reg.m5.BG_Color & 0x3F);
	MD_palette[0] = MD_palette[BG_Color];
	
	if (hs)
	{
		// Update the background color for highlight and shadow.
		
		// Normal color.
		MD_palette[192] = MD_palette[BG_Color];
		
		// Shadow color.
		MD_palette[64] = MD_palette[BG_Color + 64];
		
		// Highlight color.
		MD_palette[128] = MD_palette[BG_Color + 128];
	}
}


/**
 * VDP_Update_Palette(): Update the palette.
 */
void VDP_Update_Palette(void)
{
	T_VDP_Update_Palette<false>(MD_Palette, Palette);
	T_VDP_Update_Palette<false>(MD_Palette32, Palette32);
}

/**
 * VDP_Update_Palette_HS(): Update the palette, including highlight and shadow.
 */
void VDP_Update_Palette_HS(void)
{
	T_VDP_Update_Palette<true>(MD_Palette, Palette);
	T_VDP_Update_Palette<true>(MD_Palette32, Palette32);
}


template<typename pixel>
static inline void DrawColorBars(pixel *screen, const pixel palette[22])
{
	// Go to the correct position in the screen.
	screen += TAB336[VDP_Lines.Visible.Border_Size] + 8;
	
	// Get the horizontal pixel count.
	const int HPix = vdp_getHPix();
	
	// Pitch difference.
	const int pitch_diff = (336 - HPix);
	
	// X bar positions.
	int barX_1[7];
	for (int i = 1; i <= 7; i++)
	{
		barX_1[i-1] = ((HPix * i) / 7);
	}
	barX_1[6] = 999;
	
	int barX_2[8];
	for (int i = 1; i <= 4; i++)
	{
		barX_2[i-1] = ((HPix * i * 120) / 672);
	}
	for (int i = 1; i <= 3; i++)
	{
		barX_2[i+3] = barX_2[3] + ((HPix * i * 32) / 672);
	}
	barX_2[7] = 999;
	
	// Y bar positions.
	const int barY_1 = ((VDP_Lines.Visible.Total * 2) / 3);
	const int barY_2 = (barY_1 + (VDP_Lines.Visible.Total / 12));
	
	// Current color.
	int color;
	
	for (int y = 0; y < VDP_Lines.Visible.Total; y++)
	{
		color = 0;
		
		if (y < barY_1)
		{
			// Primary bars.
			for (int x = 0; x < HPix; x++)
			{
				if (x >= barX_1[color])
					color++;
				
				// Draw the color.
				*screen++ = palette[color];
			}
		}
		else if (y < barY_2)
		{
			// Secondary bars.
			for (int x = 0; x < HPix; x++)
			{
				if (x >= barX_1[color])
					color++;
				
				// Draw the color.
				*screen++ = palette[color+7];
			}
		}
		else
		{
			// Final bars.
			for (int x = 0; x < HPix; x++)
			{
				if (x >= barX_2[color])
					color++;
				
				// Draw the color.
				*screen++ = palette[color+14];
			}
		}
		
		// Next row.
		screen += pitch_diff;
	}
}


template<typename pixel>
static inline void DrawColorBars_Border(pixel *screen, const pixel bg_color)
{
	// Draw the top border.
	for (unsigned int i = (TAB336[VDP_Lines.Visible.Border_Size]); i != 0; i -= 4, screen += 4)
	{
		*screen = bg_color;
		*(screen + 1) = bg_color;
		*(screen + 2) = bg_color;
		*(screen + 3) = bg_color;
	}
	
	// Go to the bottom border.
	screen += (TAB336[VDP_Lines.Visible.Total]);
	
	// Draw the bottom border.
	for (unsigned int i = (TAB336[VDP_Lines.Visible.Border_Size]); i != 0; i -= 4, screen += 4)
	{
		*screen = bg_color;
		*(screen + 1) = bg_color;
		*(screen + 2) = bg_color;
		*(screen + 3) = bg_color;
	}
}


#include "video/VGA_charset.h"
template<typename pixel, pixel text_color>
static inline void DrawChr(pixel *screen, int chr)
{
	chr = (chr & 0x7F) - 0x20;
	
	// Draw the shadowed character first.
	pixel *scr_ptr = (screen + 1 + 336);
	for (unsigned int row = 0; row < 16; row++)
	{
		unsigned int chr_data = VGA_charset_ASCII[chr][row];
		if (chr_data == 0)
		{
			// Empty line.
			scr_ptr += 336;
			continue;
		}
		
		for (unsigned int col = 8; col != 0; col--, scr_ptr++)
		{
			if (chr_data & 0x80)
				*scr_ptr = 0;
			chr_data <<= 1;
		}
		
		// Next line.
		scr_ptr += (336 - 8);
	}
	
	// Draw the normal character.
	scr_ptr = screen;
	for (unsigned int row = 0; row < 16; row++)
	{
		unsigned int chr_data = VGA_charset_ASCII[chr][row];
		if (chr_data == 0)
		{
			// Empty line.
			scr_ptr += 336;
			continue;
		}
		
		for (unsigned int col = 8; col != 0; col--, scr_ptr++)
		{
			if (chr_data & 0x80)
				*scr_ptr = text_color;
			chr_data <<= 1;
		}
		
		// Next line.
		scr_ptr += (336 - 8);
	}
}

template<typename pixel, pixel text_color>
static inline void DrawText(pixel *screen, int x, int y, const char *str)
{
	pixel *scr_ptr = &screen[TAB336[y] + x + 8];
	
	for (; *str != 0x00; scr_ptr += 8, str++)
	{
		DrawChr<pixel, text_color>(scr_ptr, *str);
	}
}

template<typename pixel, pixel text_color>
static inline void DrawVDPErrorMessage(pixel *screen)
{
	// Determine the starting position.
	const int barY_1 = ((VDP_Lines.Visible.Total * 2) / 3);
	int y = VDP_Lines.Visible.Border_Size + ((barY_1 - (16*5)) / 2);
	int x = ((VDP_Reg.H_Cell - 26) / 2) * 8;
	
	DrawText<pixel, text_color>(screen, x, y,    "Gens/GS does not currently");
	DrawText<pixel, text_color>(screen, x, y+16, "  support this VDP mode.");
	y += 48;
	
	// Mode bits.
	char buf[32];
	szprintf(buf, sizeof(buf), "     Mode Bits: %d%d%d%d%d",
			(VDP_Mode & VDP_MODE_M5) >> 4,
			(VDP_Mode & VDP_MODE_M4) >> 3,
			(VDP_Mode & VDP_MODE_M3) >> 2,
			(VDP_Mode & VDP_MODE_M2) >> 1,
			(VDP_Mode & VDP_MODE_M1));
	DrawText<pixel, text_color>(screen, x, y, buf);
	
	// VDP mode.
	static const char *tms9918_modes[] =
	{
		"0 (Graphic I)",
		"1 (Text)",
		"2 (Graphic II)",
		"1+2",
		"3 (Multicolor)",
		"1+3",
		"2+3",
		"1+2+3"
	};
	const char *cur_mode;
	
	if (VDP_Mode & VDP_MODE_M4)
	{
		// Mode 4.
		cur_mode = "4 (SMS/GG)";
	}
	else
	{
		// TMS9918 mode.
		cur_mode = tms9918_modes[VDP_Mode & 0x07];
	}
	
	// Determine the horizontal starting position.
	x = ((VDP_Reg.H_Cell - (10+strlen(cur_mode))) / 2) * 8;
	DrawText<pixel, text_color>(screen, x, y+16, "VDP Mode:");
	DrawText<pixel, text_color>(screen, x+(8*10), y+16, cur_mode);
}


/**
 * VDP_Render_Line(): Render a line.
 */
void VDP_Render_Line(void)
{
	// TODO: 32X-specific function.
	
	static unsigned int Last_VDP_Mode = ~0;
	static int Last_HPix = ~0;
	static uint8_t Last_BPP = ~0;
	
	if (VDP_Mode & VDP_MODE_M5)
	{
		// Mode 5.
		VDP_Render_Line_m5();
	}
	else
	{
		// Unsupported mode.
		
		// 15-bit Color Bar colors.
		static const uint16_t cb15[22] =
		{
			// Primary Color Bars.
			0x6318, 0x6300, 0x0318, 0x0300,	// Gray, Yellow, Cyan, Green
			0x6018, 0x6000, 0x0018,		// Magenta, Red, Blue
			
			// Secondary Color Bars.
			0x0018, 0x0C63, 0x6018, 0x0C63,	// Blue, NTSC Black, Magenta, NTSC Black
			0x0318, 0x0C63, 0x6318,		// Cyan, NTSC Black, Gray
			
			// Final Color Bars.
			0x0089, 0x7FFF, 0x180D, 0x0C63,	// -I, White, +Q, NTSC Black
			0x0421, 0x0C63, 0x1084, 0x0C63,	// -4, NTSC Black, +4, NTSC Black
		};
		
		// 16-bit Color Bar colors.
		static const uint16_t cb16[22] =
		{
			// Primary Color Bars.
			0xC618, 0xC600, 0x0618, 0x600,	// Gray, Yellow, Cyan, Green
			0xC018, 0xC000, 0x0018,		// Magenta, Red, Blue
			
			// Secondary Color Bars.
			0x0018, 0x18C3, 0xC018, 0x18C3,	// Blue, NTSC Black, Magenta, NTSC Black
			0x0618, 0x18C3, 0xC618,		// Cyan, NTSC Black, Gray
			
			// Final Color Bars.
			0x0109, 0xFFFF, 0x300D, 0x18C3,	// -I, White, +Q, NTSC Black
			0x0841, 0x18C3, 0x2104, 0x18C3,	// -4, NTSC Black, +4, NTSC Black
		};
		
		// 32-bit Color Bar colors.
		static const uint32_t cb32[22] =
		{
			// Primary Color Bars.
			0xC0C0C0, 0xC0C000, 0x00C0C0, 0x00C000,	// Gray, Yellow, Cyan, Green
			0xC000C0, 0xC00000, 0x0000C0,		// Magenta, Red, Blue
			
			// Secondary Color Bars.
			0x0000C0, 0x131313, 0xC000C0, 0x131313,	// Blue, NTSC Black, Magenta, NTSC Black
			0x00C0C0, 0x131313, 0xC0C0C0,		// Cyan, NTSC Black, Gray
			
			// Final Color Bars.
			0x00214C, 0xFFFFFF, 0x32006A, 0x131313,	// -I, White, +Q, NTSC Black
			0x090909, 0x131313, 0x1D1D1D, 0x131313,	// -4, NTSC Black, +4, NTSC Black
		};
		
		if (VDP_Mode != Last_VDP_Mode ||
		    Last_HPix != vdp_getHPix() ||
		    Last_BPP != bppMD)
		{
			// VDP mode has changed. Redraw the color bars.
			if (bppMD == 15)
				DrawColorBars<uint16_t>(MD_Screen, cb15);
			else if (bppMD == 16)
				DrawColorBars<uint16_t>(MD_Screen, cb16);
			else //if (bppMD == 32)
				DrawColorBars<uint32_t>(MD_Screen32, cb32);
			
			// Force a palette update.
			VDP_Flags.CRam = 1;
		}
		
		// Check if the palette was modified.
		if (VDP_Flags.CRam)
		{
			// Update the palette.
			if (VDP_Reg.m5.Set4 & 0x08)
				VDP_Update_Palette_HS();
			else
				VDP_Update_Palette();
			
			if (VDP_Lines.Visible.Border_Size != 0)
			{
				// Update the color bar borders.
				if (bppMD != 32)
					DrawColorBars_Border<uint16_t>(MD_Screen, MD_Palette[0]);
				else //if (bppMD == 32)
					DrawColorBars_Border<uint32_t>(MD_Screen32, MD_Palette32[0]);
			}
		}
		
		// Print the error message.
		if (bppMD == 15)
			DrawVDPErrorMessage<uint16_t, 0x7FFF>(MD_Screen);
		else if (bppMD == 16)
			DrawVDPErrorMessage<uint16_t, 0xFFFF>(MD_Screen);
		else
			DrawVDPErrorMessage<uint32_t, 0xFFFFF>(MD_Screen32);
	}
	
	// Save the VDP mode.
	Last_VDP_Mode = VDP_Mode;
	Last_HPix = vdp_getHPix();
	Last_BPP = bppMD;
}
