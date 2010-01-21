/***************************************************************************
 * Gens: VDP Renderer. (Mode 5)                                            *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2010 by David Korth                                  *
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

#include "vdp_rend_m5.hpp"
#include "vdp_rend.h"
#include "vdp_io.h"

#include "emulator/g_main.hpp"

// C includes.
#include <stdint.h>
#include <string.h>


// Flickering Interlaced display testing.
// USE AT YOUR OWN RISK!
//#define FLICKERING_INTERLACED 1


// Line buffer for current line.
// TODO: Mark as static once VDP_Render_Line_m5_asm is ported to C.
// TODO: Endianness conversions.
typedef struct _LineBuf_px_t
{
	uint8_t pixel;
	uint8_t layer;
} LineBuf_px_t;
typedef union
{
	LineBuf_px_t px[336];
	uint8_t  u8[336<<1];
	uint16_t u16[336];
	uint32_t u32[336>>1];
} LineBuf_t;
LineBuf_t LineBuf;


// asm rendering code.
extern "C" void VDP_Render_Line_m5_asm(void);

VDP_Data_Misc_t VDP_Data_Misc;


/**
 * T_Make_Sprite_Struct(): Fill Sprite_Struct[] with information from the Sprite Attribute Table.
 * @param interlaced If true, using Interlaced Mode 2. (2x res)
 * @param partial If true, only do a partial update. (X pos, X size)
 */
template<bool interlaced, bool partial>
static inline void T_Make_Sprite_Struct(void)
{
	uint16_t *CurSpr = VDP_Reg.Spr_Addr;
	unsigned int spr_num = 0;
	unsigned int link;
	
	// H40 allows 80 sprites; H32 allows 64 sprites.
	// Essentially, it's (H_Cell * 2).
	const unsigned int max_spr = (VDP_Reg.H_Cell * 2);
	
	do
	{
		// Sprite position.
		Sprite_Struct[spr_num].Pos_X = (*(CurSpr + 3) & 0x1FF) - 128;
		if (!partial)
		{
			if (interlaced)
			{
				// TODO: Don't do this!
				// Use proper interlaced mode instead.
				
#ifdef FLICKERING_INTERLACED
				// Interlaced. (Flickering Interlaced is enabled.)
				Sprite_Struct[spr_num].Pos_Y = (*CurSpr & 0x3FF) - 256;
#else
				// Interlaced: Y-pos is divided by 2.
				Sprite_Struct[spr_num].Pos_Y = ((*CurSpr & 0x3FF) / 2) - 128;
#endif
			}
			else
			{
				// Non-Interlaced. Y-pos is kept as-is.
				Sprite_Struct[spr_num].Pos_Y = (*CurSpr & 0x1FF) - 128;
			}
		}
		
		// Sprite size.
		uint8_t sz = (*(CurSpr + 1) >> 8);
		Sprite_Struct[spr_num].Size_X = ((sz >> 2) & 3) + 1;	// 1 more than the original value.
		if (!partial)
			Sprite_Struct[spr_num].Size_Y = sz & 3;		// Exactly the original value.
		
		// Determine the maximum positions.
		Sprite_Struct[spr_num].Pos_X_Max =
				Sprite_Struct[spr_num].Pos_X +
				((Sprite_Struct[spr_num].Size_X * 8) - 1);
		
		if (!partial)
		{
#ifdef FLICKERING_INTERLACED
			if (interlaced)
			{
				Sprite_Struct[spr_num].Pos_Y_Max =
						Sprite_Struct[spr_num].Pos_Y +
						((Sprite_Struct[spr_num].Size_Y * 16) + 15);
			}
			else
#endif
			{
				Sprite_Struct[spr_num].Pos_Y_Max =
						Sprite_Struct[spr_num].Pos_Y +
						((Sprite_Struct[spr_num].Size_Y * 8) + 7);
			}
			
			// Tile number. (Also includes palette, priority, and flip bits.)
			Sprite_Struct[spr_num].Num_Tile = *(CurSpr + 2);
		}
		
		// Link number.
		link = (*(CurSpr + 1) & 0x7F);
		
		// Increment the sprite number.
		spr_num++;
		
		if (link == 0)
			break;
		
		// Go to the next sprite.
		CurSpr = VDP_Reg.Spr_Addr + (link * (8>>1));
		
		// Stop processing after:
		// - Link number is 0. (checked above)
		// - Link number exceeds maximum number of sprites.
		// - We've processed the maximum number of sprites.
	} while (link < max_spr && spr_num < max_spr);
	
	// Store the byte index of the last sprite.
	if (!partial)
		VDP_Data_Misc.Spr_End = (spr_num - 1) * sizeof(Sprite_Struct_t);
}


/**
 * T_Update_Mask_Sprite(): Update Sprite_Visible[] using sprite masking.
 * @param sprite_limit If true, emulates sprite limits.
 * @param interlaced If true, uses interlaced mode.
 * @return Number of visible sprites.
 */
template<bool sprite_limit, bool interlaced>
static inline unsigned int T_Update_Mask_Sprite(void)
{
	// If Sprite Limit is on, the following limits are enforced: (H32/H40)
	// - Maximum sprite dots per line: 256/320
	// - Maximum sprites per line: 16/20
	int max_cells = VDP_Reg.H_Cell;
	int max_sprites = max_cells / 2;
	
	bool overflow = false;
	
	// Sprite masking variables.
	bool sprite_on_line = false;	// True if at least one sprite is on the scanline.
	
	unsigned int spr_num = 0;	// Current sprite number in Sprite_Struct[].
	unsigned int spr_vis = 0;	// Current visible sprite in Sprite_Visible[].
	
	// Number of sprites in Sprite_Struct.
	const unsigned int TotalSprites = (VDP_Data_Misc.Spr_End / sizeof(Sprite_Struct_t)) + 1;
	
	// Get the current line number.
	int vdp_line = VDP_Lines.Visible.Current;
	
#ifdef FLICKERING_INTERLACED
	if (interlaced)
	{
		// Adjust the VDP line number for Flickering Interlaced display.
		vdp_line *= 2;
		if (VDP_Status & 0x0010)
			vdp_line++;
	}
#endif
	
	// Search for all sprites visible on the current scanline.
	for (; spr_num < TotalSprites; spr_num++)
	{
		if (Sprite_Struct[spr_num].Pos_Y > vdp_line ||
		    Sprite_Struct[spr_num].Pos_Y_Max < vdp_line)
		{
			// Sprite is not on the current line.
			continue;
		}
		
		if (sprite_limit)
		{
			max_cells -= Sprite_Struct[spr_num].Size_X;
			max_sprites--;
		}
		
		// Check sprite masking, mode 1.
		// This mode only works if at least one non-masking sprite
		// is present on the scanline, regardless of whether it's
		// visible or not.
		if (sprite_on_line && Sprite_Struct[spr_num].Pos_X == -128)
			break;
		
		// Sprite is on the current scanline.
		sprite_on_line = true;
		
		// Check if the sprite is onscreen.
		if (Sprite_Struct[spr_num].Pos_X < VDP_Reg.H_Pix &&
		    Sprite_Struct[spr_num].Pos_X_Max >= 0)
		{
			// Sprite is onscreen.
			Sprite_Visible[spr_vis] = (spr_num * sizeof(Sprite_Struct_t));
			spr_vis++;
		}
		
		if (sprite_limit && (max_cells <= 0 || max_sprites == 0))
		{
			// Sprite overflow!
			overflow = true;
			spr_num++;
			break;
		}
	}
	
	if (sprite_limit && overflow)
	{
		// Sprite overflow. Check if there are any more sprites.
		for (; spr_num < TotalSprites; spr_num++)
		{
			// Check if the sprite is on the current line.
			if (Sprite_Struct[spr_num].Pos_Y > vdp_line ||
			    Sprite_Struct[spr_num].Pos_Y_Max < vdp_line)
			{
				// Sprite is not on the current line.
				continue;
			}
			
			// Sprite is on the current line.
			// Set the SOVR flag.
			VDP_Status |= 0x40;
			break;
		}
	}
	
	// Return the number of visible sprites.
	return spr_vis;
}


/**
 * T_Get_X_Offset(): Get the X offset for the line. (Horizontal Scroll Table)
 * @param plane True for Scroll A; false for Scroll B.
 * @return X offset.
 */
template<bool plane>
static inline uint16_t T_Get_X_Offset(void)
{
	const unsigned int H_Scroll_Offset = (VDP_Lines.Visible.Current & VDP_Reg.H_Scroll_Mask) * 2;
	
	if (plane)
	{
		// Scroll A.
		return VDP_Reg.H_Scroll_Addr[H_Scroll_Offset];
	}
	else
	{
		// Scroll B.
		return VDP_Reg.H_Scroll_Addr[H_Scroll_Offset + 1];
	}
}

/**
 * C wrapper functions for T_Get_X_Offset().
 * TODO: Remove these once vdp_rend_m5_x86.asm is fully ported to C++.
 */
extern "C" {
	uint16_t Get_X_Offset_ScrollA(void);
	uint16_t Get_X_Offset_ScrollB(void);
}

uint16_t Get_X_Offset_ScrollA(void)
{
	return T_Get_X_Offset<true>();
}
uint16_t Get_X_Offset_ScrollB(void)
{
	return T_Get_X_Offset<false>();
}


/**
 * T_Update_Y_Offset(): Update the Y offset.
 * @param plane True for Scroll A; false for Scroll B.
 * @param interlaced True for interlaced; false for non-interlaced.
 * @param cell_cur Current X cell number.
 * @return Y offset.
 */
template<bool plane, bool interlaced>
static inline unsigned int T_Update_Y_Offset(int cell_cur)
{
	if ((cell_cur & 0xFF81) || (cell_cur < 0))
	{
		// Cell number is invalid.
		return 0;
	}
	
	// Get the vertical scroll offset.
	// TODO: Should cell_cur be multiplied by 2?
	// Check in Mean Bean Machine.
	unsigned int VScroll_Offset;
	if (plane)
	{
		// Scroll A.
		VScroll_Offset = VSRam.u16[cell_cur];
	}
	else
	{
		// Scroll B.
		VScroll_Offset = VSRam.u16[cell_cur + 1];
	}
	
#ifdef FLICKERING_INTERLACED
	// Flickering Interlaced mode is disabled.
	if (interlaced)
		VScroll_Offset /= 2;
	
	VScroll_Offset += VDP_Lines.Visible.Current;
	VDP_Data_Misc.Line_7 = (VScroll_Offset & 7);		// NOTE: Obsolete!
	
	// Get the V Cell offset and prevent it from overflowing.
	VScroll_Offset = (VScroll_Offset >> 3) & VDP_Reg.V_Scroll_CMask;
#else
	if (!interlaced)
	{
		// Normal mode.
		VScroll_Offset += VDP_Lines.Visible.Current;
		VDP_Data_Misc.Line_7 = (VScroll_Offset & 7);		// NOTE: Obsolete!
		VDP_Data_Misc.Y_FineOffset = (VScroll_Offset & 7);
		
		// Get the V Cell offset and prevent it from overflowing.
		VScroll_Offset = (VScroll_Offset >> 3) & VDP_Reg.V_Scroll_CMask;
	}
	else
	{
		// Interlaced mode.
		VScroll_Offset += (VDP_Lines.Visible.Current * 2);
		if (VDP_Status & 0x0010)
			VScroll_Offset++;
		
		VDP_Data_Misc.Line_7 = (VScroll_Offset >> 2) & 7;	// NOTE: Obsolete!
		VDP_Data_Misc.Y_FineOffset = (VScroll_Offset & 0x0F);
		
		// Get the V Cell offset and prevent it from overflowing.
		VScroll_Offset = (VScroll_Offset >> 4) & VDP_Reg.V_Scroll_CMask;
	}
#endif
	
	return VScroll_Offset;
}

/**
 * C wrapper functions for T_Update_Y_Offset().
 * TODO: Remove these once vdp_rend_m5_x86.asm is fully ported to C++.
 */
extern "C" {
	unsigned int Update_Y_Offset_ScrollA(unsigned int cur);
	unsigned int Update_Y_Offset_ScrollB(unsigned int cur);
	unsigned int Update_Y_Offset_ScrollA_Interlaced(unsigned int cur);
	unsigned int Update_Y_Offset_ScrollB_Interlaced(unsigned int cur);
}

unsigned int Update_Y_Offset_ScrollA(unsigned int cur)
{ return T_Update_Y_Offset<true, false>(cur); }
unsigned int Update_Y_Offset_ScrollB(unsigned int cur)
{ return T_Update_Y_Offset<false, false>(cur); }
unsigned int Update_Y_Offset_ScrollA_Interlaced(unsigned int cur)
{ return T_Update_Y_Offset<true, true>(cur); }
unsigned int Update_Y_Offset_ScrollB_Interlaced(unsigned int cur)
{ return T_Update_Y_Offset<false, true>(cur); }


/**
 * T_Get_Pattern_Info(): Get pattern info from a scroll plane.
 * H_Scroll_CMul must be initialized correctly.
 * @param plane True for Scroll A; false for Scroll B.
 * @param x X tile number.
 * @param y Y tile number.
 * @return Pattern info.
 */
template<bool plane>
static inline uint16_t T_Get_Pattern_Info(unsigned int x, unsigned int y)
{
	// Get the offset.
	// H_Scroll_CMul is the shift value required for the proper vertical offset.
	unsigned int offset = (y << VDP_Reg.H_Scroll_CMul) + x;
	
	// Return the pattern information.
	return (plane ? VDP_Reg.ScrA_Addr[offset] : VDP_Reg.ScrB_Addr[offset]);
}

/**
 * C wrapper functions for T_Get_Pattern_Info().
 * TODO: Remove these once vdp_rend_m5_x86.asm is fully ported to C++.
 */
extern "C" {
	uint16_t Get_Pattern_Info_ScrollA(unsigned int x, unsigned int y);
	uint16_t Get_Pattern_Info_ScrollB(unsigned int x, unsigned int y);
}

uint16_t Get_Pattern_Info_ScrollA(unsigned int x, unsigned int y)
{
	return T_Get_Pattern_Info<true>(x, y);
}
uint16_t Get_Pattern_Info_ScrollB(unsigned int x, unsigned int y)
{
	return T_Get_Pattern_Info<false>(x, y);
}


/**
 * Get_Pattern_Info(): Get pattern info for the window.
 * H_Scroll_CMul must be initialized correctly.
 * @param x X tile number.
 * @param y Y tile number.
 * @return Pattern info.
 */
static inline uint16_t Get_Pattern_Info_Window(unsigned int x, unsigned int y)
{
	// Get the offset.
	// H_Scroll_CMul is the shift value required for the proper vertical offset.
	unsigned int offset = (y << VDP_Reg.H_Scroll_CMul) + x;
	
	// Return the pattern information.
	return VDP_Reg.Win_Addr[offset];
}


/**
 * T_Get_Pattern_Data(): Get pattern data for a given tile for the current line.
 * @param interlaced True for interlaced; false for non-interlaced.
 * @param pattern Pattern info.
 * @return Pattern data.
 */
template<bool interlaced>
static inline unsigned int T_Get_Pattern_Data(uint16_t pattern)
{
	// Vertical offset.
#ifdef FLICKERING_INTERLACED
	// TODO: Switch to VDP_Data_Misc.Y_FineOffset exclusively once Scroll A is ported to C++.
	unsigned int V_Offset;
	if (interlaced)
		V_Offset = VDP_Data_Misc.Y_FineOffset;
	else
		V_Offset = VDP_Data_Misc.Line_7;
#else
	unsigned int V_Offset = VDP_Data_Misc.Line_7;
#endif
	
	// Get the tile address.
	unsigned int TileAddr;
	if (interlaced)
		TileAddr = (pattern & 0x3FF) << 6;
	else
		TileAddr = (pattern & 0x7FF) << 5;
	
	if (pattern & 0x1000)
	{
		// V Flip enabled. Flip the tile vertically.
#ifdef FLICKERING_INTERLACED
		if (interlaced)
			V_Offset ^= 15;
		else
#endif
			V_Offset ^= 7;
	}
	
#ifndef FLICKERING_INTERLACED
	if (interlaced)
		return VRam.u32[(TileAddr + (V_Offset * 8)) >> 2];
	else
#endif
		return VRam.u32[(TileAddr + (V_Offset * 4)) >> 2];
}

/**
 * C wrapper functions for T_Get_Pattern_Data().
 * TODO: Remove these once vdp_rend_m5_x86.asm is fully ported to C++.
 */
extern "C" {
	unsigned int Get_Pattern_Data(uint16_t pattern);
	unsigned int Get_Pattern_Data_Interlaced(uint16_t pattern);
}

unsigned int Get_Pattern_Data(uint16_t pattern)
{
	return T_Get_Pattern_Data<false>(pattern);
}
unsigned int Get_Pattern_Data_Interlaced(uint16_t pattern)
{
	return T_Get_Pattern_Data<true>(pattern);
}


#define LINEBUF_HIGH_B	0x80
#define LINEBUF_SHAD_B	0x40
#define LINEBUF_PRIO_B	0x01
#define LINEBUF_SPR_B	0x20
#define LINEBUF_WIN_B	0x02

#define LINEBUF_HIGH_W	0x8080
#define LINEBUF_SHAD_W	0x4040
#define LINEBUF_PRIO_W	0x0100
#define LINEBUF_SPR_W	0x2000
#define LINEBUF_WIN_W	0x0200

/**
 * T_PutPixel_P0(): Put a pixel in background graphics layer 0.
 * @param plane		[in] True for Scroll A; false for Scroll B.
 * @param h_s		[in] Highlight/Shadow enable.
 * @param pat_pixnum	[in] Pattern pixel number.
 * @param mask		[in] Mask to isolate the good pixel.
 * @param shift		[in] Shift.
 * @param disp_pixnum	[in] Display pixel number.
 * @param pattern	[in] Pattern data.
 * @param palette	[in] Palette number * 16.
 */
template<bool plane, bool h_s, int pat_pixnum, uint32_t mask, int shift>
static inline void T_PutPixel_P0(int disp_pixnum, uint32_t pattern, unsigned int palette)
{
	// TODO: Convert mask and shift to template parameters.
	
	// Check if this is a transparent pixel.
	if (!(pattern & mask))
		return;
	
	// Check the layer bits of the current pixel.
	const unsigned int LineBuf_pixnum = (disp_pixnum + pat_pixnum);
	uint8_t layer_bits = LineBuf.px[LineBuf_pixnum].layer;
	if (plane && (layer_bits & (LINEBUF_PRIO_B | LINEBUF_WIN_B)))
	{
		// Scroll A: Either the pixel has priority set,
		// or the pixel is a window pixel.
		return;
	}
	
	// Shift the pattern data.
	uint8_t pat8 = (pattern >> shift) & 0x0F;
	
	// Apply palette data.
	pat8 |= palette;
	
	// If Highlight/Shadow is enabled, mark this pixel as shadow.
	if (h_s)
		pat8 |= LINEBUF_SHAD_B;
	
	// Write the new pixel to the line buffer.
	LineBuf.px[LineBuf_pixnum].pixel = pat8;
}


/**
 * T_PutPixel_P1(): Put a pixel in background graphics layer 1.
 * @param plane		[in] True for Scroll A; false for Scroll B.
 * @param h_s		[in] Highlight/Shadow enable.
 * @param pat_pixnum	[in] Pattern pixel number.
 * @param mask		[in] Mask to isolate the good pixel.
 * @param shift		[in] Shift.
 * @param disp_pixnum	[in] Display pixel number.
 * @param pattern	[in] Pattern data.
 * @param palette	[in] Palette number * 16.
 */
template<bool plane, bool h_s, int pat_pixnum, uint32_t mask, int shift>
static inline void T_PutPixel_P1(int disp_pixnum, uint32_t pattern, unsigned int palette)
{
	// TODO: Convert mask and shift to template parameters.
	
	// Check if this is a transparent pixel.
	unsigned int px = (pattern & mask);
	if (px == 0)
		return;
	
	const unsigned int LineBuf_pixnum = (disp_pixnum + pat_pixnum);
	
	if (plane)
	{
		// Scroll A: If the pixel is a Window pixel, don't do anything.
		if (LineBuf.px[LineBuf_pixnum].layer & LINEBUF_WIN_B)
			return;
	}
	
	// Shift the pixel.
	px >>= shift;
	
	// Update the pixel:
	// - Add palette information.
	// - Mark the pixel as priority.
	// - Save it to the linebuffer.
	px |= palette | LINEBUF_PRIO_W;
	LineBuf.u16[LineBuf_pixnum] = (uint16_t)px;
}


/**
 * T_PutPixel_Sprite(): Put a pixel in the sprite layer.
 * @param priority	[in] Sprite priority.
 * @param h_s		[in] Highlight/Shadow enable.
 * @param pat_pixnum	[in] Pattern pixel number.
 * @param mask		[in] Mask to isolate the good pixel.
 * @param shift		[in] Shift.
 * @param disp_pixnum	[in] Display pixel number.
 * @param pattern	[in] Pattern data.
 * @param palette	[in] Palette number * 16.
 * @return Linebuffer byte.
 */
template<bool priority, bool h_s, int pat_pixnum, uint32_t mask, int shift>
static inline uint8_t T_PutPixel_Sprite(int disp_pixnum, uint32_t pattern, unsigned int palette)
{
	// TODO: Convert mask and shift to template parameters.
	
	// Check if this is a transparent pixel.
	unsigned int px = (pattern & mask);
	if (px == 0)
		return 0;
	
	// Get the pixel number in the linebuffer.
	const unsigned int LineBuf_pixnum = (disp_pixnum + pat_pixnum + 8);
	
	// TODO: Endianness conversions.
	uint8_t layer_bits = LineBuf.px[LineBuf_pixnum].layer;
	
	if (layer_bits & (LINEBUF_PRIO_B + LINEBUF_SPR_B - priority))
	{
		// Priority bit is set. (TODO: Is that what this means?)
		if (!priority)
		{
			// Set the sprite bit in the linebuffer.
			LineBuf.px[LineBuf_pixnum].layer |= LINEBUF_SPR_B;
		}
		
		// Return the original linebuffer priority data.
		return layer_bits;
	}
	
	// Shift the pixel and apply the palette.
	px = ((px >> shift) | palette);
	
	if (h_s)
	{
		// Highlight/Shadow enabled.
		if (px == 0x3E)
		{
			// Palette 3, color 14: Highlight. (Sprite pixel doesn't show up.)
			LineBuf.u16[LineBuf_pixnum] |= LINEBUF_HIGH_W;
			return 0;
		}
		else if (px == 0x3F)
		{
			// Palette 3, color 15: Shadow. (Sprite pixel doesn't show up.)
			LineBuf.u16[LineBuf_pixnum] |= LINEBUF_SHAD_W;
			return 0;
		}
		
		// Apply highlight/shadow based on priority.
		if (!priority)
			layer_bits &= (LINEBUF_SHAD_B | LINEBUF_HIGH_B);
		else
			layer_bits &= LINEBUF_HIGH_B;
		
		// Apply the layer bits.
		px |= layer_bits;
	}
	
	// Mark the pixel as a sprite pixel.
	px |= LINEBUF_SPR_W;
	
	// Save the pixel in the linebuffer.
	LineBuf.u16[LineBuf_pixnum] = px;
	
	return 0;
}


#define LINEBUF_HIGH_D	0x80808080
#define LINEBUF_SHAD_D	0x40404040
#define LINEBUF_PRIO_D	0x01000100
#define LINEBUF_SPR_D	0x20002000
#define LINEBUF_WIN_D	0x02000200

/**
 * T_PutLine_P0(): Put a line in background graphics layer 0.
 * @param plane		[in] True for Scroll A; false for Scroll B.
 * @param h_s		[in] Highlight/Shadow enable.
 * @param flip		[in] True to flip the line horizontally.
 * @param disp_pixnum	[in] Display pixel nmber.
 * @param pattern	[in] Pattern data.
 * @param palette	[in] Palette number * 16.
 */
template<bool plane, bool h_s, bool flip>
static inline void T_PutLine_P0(int disp_pixnum, uint32_t pattern, int palette)
{
	if (!plane)
	{
		// Scroll B.
		if (h_s)
		{
			// Highlight/Shadow is enabled.
			// Set the line to shadow by default.
			memset(&LineBuf.u16[disp_pixnum], LINEBUF_SHAD_B, 8*2);
		}
		else
		{
			// Highlight/Shadow is disabled.
			// Clear the line.
			memset(&LineBuf.u16[disp_pixnum], 0x00, 8*2);
		}
		
		// If ScrollB_Low is disabled, don't do anything.
		if (!(VDP_Layers & VDP_LAYER_SCROLLB_LOW))
			return;
	}
	else
	{
		// Scroll A.
		// If ScrollA Low is disabled. don't do anything.
		if (!(VDP_Layers & VDP_LAYER_SCROLLA_LOW))
			return;
	}
	
	// Don't do anything if the pattern is empty.
	if (pattern == 0)
		return;
	
	// Put the pixels.
	if (!flip)
	{
		// No flip.
		T_PutPixel_P0<plane, h_s, 0, 0x0000F000, 12>(disp_pixnum, pattern, palette);
		T_PutPixel_P0<plane, h_s, 1, 0x00000F00,  8>(disp_pixnum, pattern, palette);
		T_PutPixel_P0<plane, h_s, 2, 0x000000F0,  4>(disp_pixnum, pattern, palette);
		T_PutPixel_P0<plane, h_s, 3, 0x0000000F,  0>(disp_pixnum, pattern, palette);
		T_PutPixel_P0<plane, h_s, 4, 0xF0000000, 28>(disp_pixnum, pattern, palette);
		T_PutPixel_P0<plane, h_s, 5, 0x0F000000, 24>(disp_pixnum, pattern, palette);
		T_PutPixel_P0<plane, h_s, 6, 0x00F00000, 20>(disp_pixnum, pattern, palette);
		T_PutPixel_P0<plane, h_s, 7, 0x000F0000, 16>(disp_pixnum, pattern, palette);
	}
	else
	{
		// Horizontal flip.
		T_PutPixel_P0<plane, h_s, 0, 0x000F0000, 16>(disp_pixnum, pattern, palette);
		T_PutPixel_P0<plane, h_s, 1, 0x00F00000, 20>(disp_pixnum, pattern, palette);
		T_PutPixel_P0<plane, h_s, 2, 0x0F000000, 24>(disp_pixnum, pattern, palette);
		T_PutPixel_P0<plane, h_s, 3, 0xF0000000, 28>(disp_pixnum, pattern, palette);
		T_PutPixel_P0<plane, h_s, 4, 0x0000000F,  0>(disp_pixnum, pattern, palette);
		T_PutPixel_P0<plane, h_s, 5, 0x000000F0,  4>(disp_pixnum, pattern, palette);
		T_PutPixel_P0<plane, h_s, 6, 0x00000F00,  8>(disp_pixnum, pattern, palette);
		T_PutPixel_P0<plane, h_s, 7, 0x0000F000, 12>(disp_pixnum, pattern, palette);
	}
}

/**
 * C wrapper functions for T_PutLine_P0().
 * TODO: Remove these once vdp_rend_m5_x86.asm is fully ported to C++.
 */
extern "C" {
	void PutLine_P0_ScrollA(int disp_pixnum, uint32_t pattern, int palette);
	void PutLine_P0_ScrollA_HS(int disp_pixnum, uint32_t pattern, int palette);
	void PutLine_P0_ScrollB(int disp_pixnum, uint32_t pattern, int palette);
	void PutLine_P0_ScrollB_HS(int disp_pixnum, uint32_t pattern, int palette);
	void PutLine_P0_Flip_ScrollA(int disp_pixnum, uint32_t pattern, int palette);
	void PutLine_P0_Flip_ScrollA_HS(int disp_pixnum, uint32_t pattern, int palette);
	void PutLine_P0_Flip_ScrollB(int disp_pixnum, uint32_t pattern, int palette);
	void PutLine_P0_Flip_ScrollB_HS(int disp_pixnum, uint32_t pattern, int palette);
}

void PutLine_P0_ScrollA(int disp_pixnum, uint32_t pattern, int palette)
{
	T_PutLine_P0<true, false, false>(disp_pixnum, pattern, palette);
}
void PutLine_P0_ScrollA_HS(int disp_pixnum, uint32_t pattern, int palette)
{
	T_PutLine_P0<true, true, false>(disp_pixnum, pattern, palette);
}
void PutLine_P0_ScrollB(int disp_pixnum, uint32_t pattern, int palette)
{
	T_PutLine_P0<false, false, false>(disp_pixnum, pattern, palette);
}
void PutLine_P0_ScrollB_HS(int disp_pixnum, uint32_t pattern, int palette)
{
	T_PutLine_P0<false, true, false>(disp_pixnum, pattern, palette);
}
void PutLine_P0_Flip_ScrollA(int disp_pixnum, uint32_t pattern, int palette)
{
	T_PutLine_P0<true, false, true>(disp_pixnum, pattern, palette);
}
void PutLine_P0_Flip_ScrollA_HS(int disp_pixnum, uint32_t pattern, int palette)
{
	T_PutLine_P0<true, true, true>(disp_pixnum, pattern, palette);
}
void PutLine_P0_Flip_ScrollB(int disp_pixnum, uint32_t pattern, int palette)
{
	T_PutLine_P0<false, false, true>(disp_pixnum, pattern, palette);
}
void PutLine_P0_Flip_ScrollB_HS(int disp_pixnum, uint32_t pattern, int palette)
{
	T_PutLine_P0<false, true, true>(disp_pixnum, pattern, palette);
}


/**
 * T_PutLine_P1(): Put a line in background graphics layer 1.
 * @param plane		[in] True for Scroll A; false for Scroll B.
 * @param h_s		[in] Highlight/Shadow enable.
 * @param flip		[in] True to flip the line horizontally.
 * @param disp_pixnum	[in] Display pixel nmber.
 * @param pattern	[in] Pattern data.
 * @param palette	[in] Palette number * 16.
 */
template<bool plane, bool h_s, bool flip>
static inline void T_PutLine_P1(int disp_pixnum, uint32_t pattern, int palette)
{
	if (!plane)
	{
		// Scroll B.
		// Clear the line.
		memset(&LineBuf.u16[disp_pixnum], 0x00, 8*2);
		
		// If ScrollB_Low is disabled, don't do anything.
		if (!(VDP_Layers & VDP_LAYER_SCROLLB_LOW))
			return;
	}
	else
	{
		// Scroll A.
		// If ScrollA Low is disabled. don't do anything.
		if (!(VDP_Layers & VDP_LAYER_SCROLLA_LOW))
			return;
		
		// AND the linebuffer with ~LINEBUF_SHAD_W.
		// TODO: Optimize this to use 32-bit operations instead of 16-bit.
		LineBuf.u16[disp_pixnum]   &= ~LINEBUF_SHAD_W;
		LineBuf.u16[disp_pixnum+1] &= ~LINEBUF_SHAD_W;
		LineBuf.u16[disp_pixnum+2] &= ~LINEBUF_SHAD_W;
		LineBuf.u16[disp_pixnum+3] &= ~LINEBUF_SHAD_W;
		LineBuf.u16[disp_pixnum+4] &= ~LINEBUF_SHAD_W;
		LineBuf.u16[disp_pixnum+5] &= ~LINEBUF_SHAD_W;
		LineBuf.u16[disp_pixnum+6] &= ~LINEBUF_SHAD_W;
		LineBuf.u16[disp_pixnum+7] &= ~LINEBUF_SHAD_W;
	}
	
	// Don't do anything if the pattern is empty.
	if (pattern == 0)
		return;
	
	// Put the pixels.
	if (!flip)
	{
		// No flip.
		T_PutPixel_P1<plane, h_s, 0, 0x0000F000, 12>(disp_pixnum, pattern, palette);
		T_PutPixel_P1<plane, h_s, 1, 0x00000F00,  8>(disp_pixnum, pattern, palette);
		T_PutPixel_P1<plane, h_s, 2, 0x000000F0,  4>(disp_pixnum, pattern, palette);
		T_PutPixel_P1<plane, h_s, 3, 0x0000000F,  0>(disp_pixnum, pattern, palette);
		T_PutPixel_P1<plane, h_s, 4, 0xF0000000, 28>(disp_pixnum, pattern, palette);
		T_PutPixel_P1<plane, h_s, 5, 0x0F000000, 24>(disp_pixnum, pattern, palette);
		T_PutPixel_P1<plane, h_s, 6, 0x00F00000, 20>(disp_pixnum, pattern, palette);
		T_PutPixel_P1<plane, h_s, 7, 0x000F0000, 16>(disp_pixnum, pattern, palette);
	}
	else
	{
		// Horizontal flip.
		T_PutPixel_P1<plane, h_s, 0, 0x000F0000, 16>(disp_pixnum, pattern, palette);
		T_PutPixel_P1<plane, h_s, 1, 0x00F00000, 20>(disp_pixnum, pattern, palette);
		T_PutPixel_P1<plane, h_s, 2, 0x0F000000, 24>(disp_pixnum, pattern, palette);
		T_PutPixel_P1<plane, h_s, 3, 0xF0000000, 28>(disp_pixnum, pattern, palette);
		T_PutPixel_P1<plane, h_s, 4, 0x0000000F,  0>(disp_pixnum, pattern, palette);
		T_PutPixel_P1<plane, h_s, 5, 0x000000F0,  4>(disp_pixnum, pattern, palette);
		T_PutPixel_P1<plane, h_s, 6, 0x00000F00,  8>(disp_pixnum, pattern, palette);
		T_PutPixel_P1<plane, h_s, 7, 0x0000F000, 12>(disp_pixnum, pattern, palette);
	}
}

/**
 * C wrapper functions for T_PutLine_P1().
 * TODO: Remove these once vdp_rend_m5_x86.asm is fully ported to C++.
 */
extern "C" {
	void PutLine_P1_ScrollA(int disp_pixnum, uint32_t pattern, int palette);
	void PutLine_P1_ScrollA_HS(int disp_pixnum, uint32_t pattern, int palette);
	void PutLine_P1_ScrollB(int disp_pixnum, uint32_t pattern, int palette);
	void PutLine_P1_ScrollB_HS(int disp_pixnum, uint32_t pattern, int palette);
	void PutLine_P1_Flip_ScrollA(int disp_pixnum, uint32_t pattern, int palette);
	void PutLine_P1_Flip_ScrollA_HS(int disp_pixnum, uint32_t pattern, int palette);
	void PutLine_P1_Flip_ScrollB(int disp_pixnum, uint32_t pattern, int palette);
	void PutLine_P1_Flip_ScrollB_HS(int disp_pixnum, uint32_t pattern, int palette);
}

void PutLine_P1_ScrollA(int disp_pixnum, uint32_t pattern, int palette)
{
	T_PutLine_P1<true, false, false>(disp_pixnum, pattern, palette);
}
void PutLine_P1_ScrollA_HS(int disp_pixnum, uint32_t pattern, int palette)
{
	T_PutLine_P1<true, true, false>(disp_pixnum, pattern, palette);
}
void PutLine_P1_ScrollB(int disp_pixnum, uint32_t pattern, int palette)
{
	T_PutLine_P1<false, false, false>(disp_pixnum, pattern, palette);
}
void PutLine_P1_ScrollB_HS(int disp_pixnum, uint32_t pattern, int palette)
{
	T_PutLine_P1<false, true, false>(disp_pixnum, pattern, palette);
}
void PutLine_P1_Flip_ScrollA(int disp_pixnum, uint32_t pattern, int palette)
{
	T_PutLine_P1<true, false, true>(disp_pixnum, pattern, palette);
}
void PutLine_P1_Flip_ScrollA_HS(int disp_pixnum, uint32_t pattern, int palette)
{
	T_PutLine_P1<true, true, true>(disp_pixnum, pattern, palette);
}
void PutLine_P1_Flip_ScrollB(int disp_pixnum, uint32_t pattern, int palette)
{
	T_PutLine_P1<false, false, true>(disp_pixnum, pattern, palette);
}
void PutLine_P1_Flip_ScrollB_HS(int disp_pixnum, uint32_t pattern, int palette)
{
	T_PutLine_P1<false, true, true>(disp_pixnum, pattern, palette);
}


/**
 * T_PutLine_Sprite(): Put a line in the sprite layer.
 * @param priority	[in] Sprite priority. (false == low, true == high)
 * @param h_s		[in] Highlight/Shadow enable.
 * @param flip		[in] True to flip the line horizontally.
 * @param disp_pixnum	[in] Display pixel nmber.
 * @param pattern	[in] Pattern data.
 * @param palette	[in] Palette number * 16.
 */
template<bool priority, bool h_s, bool flip>
static inline void T_PutLine_Sprite(int disp_pixnum, uint32_t pattern, int palette)
{
	// Check if the sprite layer is disabled.
	if (!(VDP_Layers & (priority ? VDP_LAYER_SPRITE_HIGH : VDP_LAYER_SPRITE_LOW)))
	{
		// Sprite layer is disabled.
		return;
	}
	
	// Put the sprite pixels.
	uint8_t status = 0;
	if (!flip)
	{
		// No flip.
		status |= T_PutPixel_Sprite<priority, h_s, 0, 0x0000F000, 12>(disp_pixnum, pattern, palette);
		status |= T_PutPixel_Sprite<priority, h_s, 1, 0x00000F00,  8>(disp_pixnum, pattern, palette);
		status |= T_PutPixel_Sprite<priority, h_s, 2, 0x000000F0,  4>(disp_pixnum, pattern, palette);
		status |= T_PutPixel_Sprite<priority, h_s, 3, 0x0000000F,  0>(disp_pixnum, pattern, palette);
		status |= T_PutPixel_Sprite<priority, h_s, 4, 0xF0000000, 28>(disp_pixnum, pattern, palette);
		status |= T_PutPixel_Sprite<priority, h_s, 5, 0x0F000000, 24>(disp_pixnum, pattern, palette);
		status |= T_PutPixel_Sprite<priority, h_s, 6, 0x00F00000, 20>(disp_pixnum, pattern, palette);
		status |= T_PutPixel_Sprite<priority, h_s, 7, 0x000F0000, 16>(disp_pixnum, pattern, palette);
	}
	else
	{
		// Horizontal flip.
		status |= T_PutPixel_Sprite<priority, h_s, 0, 0x000F0000, 16>(disp_pixnum, pattern, palette);
		status |= T_PutPixel_Sprite<priority, h_s, 1, 0x00F00000, 20>(disp_pixnum, pattern, palette);
		status |= T_PutPixel_Sprite<priority, h_s, 2, 0x0F000000, 24>(disp_pixnum, pattern, palette);
		status |= T_PutPixel_Sprite<priority, h_s, 3, 0xF0000000, 28>(disp_pixnum, pattern, palette);
		status |= T_PutPixel_Sprite<priority, h_s, 4, 0x0000000F,  0>(disp_pixnum, pattern, palette);
		status |= T_PutPixel_Sprite<priority, h_s, 5, 0x000000F0,  4>(disp_pixnum, pattern, palette);
		status |= T_PutPixel_Sprite<priority, h_s, 6, 0x00000F00,  8>(disp_pixnum, pattern, palette);
		status |= T_PutPixel_Sprite<priority, h_s, 7, 0x0000F000, 12>(disp_pixnum, pattern, palette);
	}
	
	// Check for sprite collision.
	VDP_Status |= (status & 0x20);
}


/**
 * T_Render_Line_Scroll(): Render a scroll line.
 * @param plane		[in] True for Scroll A / Window; false for Scroll B.
 * @param interlaced	[in] True for interlaced; false for non-interlaced.
 * @param vscroll	[in] True for 2-cell mode; false for full scroll.
 * @param h_s		[in] Highlight/Shadow enable.
 * @param cell_start	[in] (Scroll A) First cell to draw.
 * @param cell_length	[in] (Scroll A) Number of cells to draw.
 */
template<bool plane, bool interlaced, bool vscroll, bool h_s>
static inline void T_Render_Line_Scroll(int cell_start, int cell_length)
{
	// TODO: For Scroll A, only render non-window areas.
	
	// Get the horizontal scroll offset. (cell and fine offset)
	unsigned int X_offset_cell = T_Get_X_Offset<plane>() & 0x3FF;
	
	// Drawing will start at the fine cell offset.
	// LineBuf.u16[X_offset_cell & 7]
	unsigned int disp_pixnum = (X_offset_cell & 7);
	
	// Determine if we should apply the Left Window bug.
	int LeftWindowBugCnt = 0;	// Left Window bug counter.
	if (plane && (cell_start != 0))
	{
		// Determine the value for the Left Window bug counter.
		// First tile: Counter should be 2.
		// Second tile: Counter should be 1.
		LeftWindowBugCnt = ((X_offset_cell & 8) ? 2 : 1);
	}
	
	if (plane)
	{
		// Adjust for the cell starting position.
		const int cell_start_px = (cell_start << 3);
		X_offset_cell -= cell_start_px;
		disp_pixnum += cell_start_px;
	}
	
	// Get the correct cell offset:
	// - Invert the cell position.
	// - Right-shift by 3 for the cell number.
	// - AND with the horizontal scrolling cell mask to prevent overflow.
	X_offset_cell = (((X_offset_cell ^ 0x3FF) >> 3) & VDP_Reg.H_Scroll_CMask);
	
	int cell_cur = (plane ? 0 : cell_start);	// Current cell number.
	
	// Initialize the Y offset.
	unsigned int Y_offset_cell;
	if (!vscroll)
	{
		// Full vertical scrolling.
		// Initialize the Y offset here.
		Y_offset_cell = T_Update_Y_Offset<plane, interlaced>(cell_cur);
	}
	
	// Loop through the cells.
	for (int x = (plane ? cell_length : VDP_Reg.H_Cell);
	     x >= 0; x--, cell_cur++)
	{
		if (vscroll)
		{
			// 2-cell vertical scrolling.
			// Update the Y offset.
			Y_offset_cell = T_Update_Y_Offset<plane, interlaced>(cell_cur);
		}
		
		// Get the pattern info for the current tile.
		uint32_t pattern_info;
		if (!plane)
		{
			// Scroll B.
			pattern_info = T_Get_Pattern_Info<plane>(X_offset_cell, Y_offset_cell);
		}
		else
		{
			// Scroll A. Check if we need to emulate the Left Window bug.
			if (LeftWindowBugCnt <= 0)
			{
				// Left Window bug doesn't apply or has already been emulated.
				pattern_info = T_Get_Pattern_Info<plane>(X_offset_cell, Y_offset_cell);
			}
			else
			{
				// Left Window bug applies.
				LeftWindowBugCnt--;
				const unsigned int TmpXCell = ((X_offset_cell + 2) & VDP_Reg.H_Scroll_CMask);
				pattern_info = T_Get_Pattern_Info<plane>(TmpXCell, Y_offset_cell);
			}
		}
		
		// Get the pattern data for the current tile.
		uint32_t pattern_data = T_Get_Pattern_Data<interlaced>(pattern_info);
		
		// Extract the palette number.
		// Resulting number is palette * 16.
		unsigned int palette = (pattern_info >> 9) & 0x30;
		
		// Check for swapped Scroll B priority.
		if (VDP_Layers & VDP_LAYER_SCROLLB_SWAP)
			pattern_info ^= 0x8000;
		
		// Check for horizontal flip.
		if (pattern_info & 0x0800)
		{
			// Pattern has H-Flip enabled.
			if (pattern_info & 0x8000)
				T_PutLine_P1<plane, h_s, true>(disp_pixnum, pattern_data, palette);
			else
				T_PutLine_P0<plane, h_s, true>(disp_pixnum, pattern_data, palette);
		}
		else
		{
			// Pattern doesn't have flip enabled.
			if (pattern_info & 0x8000)
				T_PutLine_P1<plane, h_s, false>(disp_pixnum, pattern_data, palette);
			else
				T_PutLine_P0<plane, h_s, false>(disp_pixnum, pattern_data, palette);
		}
		
		// Go to the next H cell.
		X_offset_cell = (X_offset_cell + 1) & VDP_Reg.H_Scroll_CMask;
		
		// Go to the next pattern.
		disp_pixnum += 8;
	}
}

/**
 * C wrapper functions for T_Render_Line_ScrollB().
 * TODO: Remove these once vdp_rend_m5_x86.asm is fully ported to C++.
 */
extern "C" {
	void Render_Line_ScrollB(void);
	void Render_Line_ScrollB_Interlaced(void);
	void Render_Line_ScrollB_VScroll(void);
	void Render_Line_ScrollB_VScroll_Interlaced(void);
	void Render_Line_ScrollB_HS(void);
	void Render_Line_ScrollB_HS_Interlaced(void);
	void Render_Line_ScrollB_HS_VScroll(void);
	void Render_Line_ScrollB_HS_VScroll_Interlaced(void);
}
void Render_Line_ScrollB(void)
{ T_Render_Line_Scroll<false, false, false, false>(0, 0); }
void Render_Line_ScrollB_Interlaced(void)
{ T_Render_Line_Scroll<false, true, false, false>(0, 0); }
void Render_Line_ScrollB_VScroll(void)
{ T_Render_Line_Scroll<false, false, true, false>(0, 0); }
void Render_Line_ScrollB_VScroll_Interlaced(void)
{ T_Render_Line_Scroll<false, true, true, false>(0, 0); }
void Render_Line_ScrollB_HS(void)
{ T_Render_Line_Scroll<false, false, false, true>(0, 0); }
void Render_Line_ScrollB_HS_Interlaced(void)
{ T_Render_Line_Scroll<false, true, false, true>(0, 0); }
void Render_Line_ScrollB_HS_VScroll(void)
{ T_Render_Line_Scroll<false, false, true, true>(0, 0); }
void Render_Line_ScrollB_HS_VScroll_Interlaced(void)
{ T_Render_Line_Scroll<false, true, true, true>(0, 0); }


/**
 * T_Render_Line_ScrollA(): Render a line for Scroll A / Window.
 * @param interlaced	[in] True for interlaced; false for non-interlaced.
 * @param vscroll	[in] True for 2-cell mode; false for full scroll.
 * @param h_s		[in] Highlight/Shadow enable.
 */
template<bool interlaced, bool vscroll, bool h_s>
static inline void T_Render_Line_ScrollA(void)
{
	// TODO: Window code.
	// For now, we'll only draw Scroll A.
	
	// Cell counts for Scroll A.
	int ScrA_Start, ScrA_Length;
	int Win_Start, Win_Length = 0;
	
	// Check if the entire line is part of the window.
	// TODO: Verify interlaced operation!
	const int vdp_cells = (VDP_Lines.Visible.Current >> 3);
	if (VDP_Reg.m5.Win_V_Pos & 0x80)
	{
		// Window starts from the bottom.
		if (vdp_cells >= VDP_Reg.Win_Y_Pos)
		{
			// Current line is >= starting line.
			// Entire line is part of the window.
			ScrA_Start = 0;
			ScrA_Length = 0;
			Win_Start = 0;
			Win_Length = VDP_Reg.H_Cell;
		}
	}
	else if (vdp_cells < VDP_Reg.Win_Y_Pos)
	{
		// Current line is < ending line.
		// Entire line is part of the window.
		ScrA_Start = 0;
		ScrA_Length = 0;
		Win_Start = 0;
		Win_Length = VDP_Reg.H_Cell;
	}
	
	if (Win_Length == 0)
	{
		// Determine the cell starting position and length.
		if (VDP_Reg.m5.Win_H_Pos & 0x80)
		{
			// Window is right-aligned.
			ScrA_Start = 0;
			ScrA_Length = VDP_Reg.Win_X_Pos;
			Win_Start = VDP_Reg.Win_X_Pos;
			Win_Length = (VDP_Reg.H_Cell - VDP_Reg.Win_X_Pos);
		}
		else
		{
			// Window is left-aligned.
			Win_Start = 0;
			Win_Length = VDP_Reg.Win_X_Pos;
			ScrA_Start = VDP_Reg.Win_X_Pos;
			ScrA_Length = (VDP_Reg.H_Cell - VDP_Reg.Win_X_Pos);
		}
	}
	
	if (Win_Length > 0)
	{
		// Mark window pixels.
		const int StartPx = ((Win_Start * 8) + 8) / 2;
		const int EndPx = StartPx + ((Win_Length * 8) / 2);
		
		for (int x = StartPx; x < EndPx; x++)
			LineBuf.u32[x] |= LINEBUF_WIN_D;
	}
	
	// Draw the scroll area.
	T_Render_Line_Scroll<true, interlaced, vscroll, h_s>(ScrA_Start, ScrA_Length);
}

/**
 * C wrapper functions for T_Render_Line_ScrollA().
 * TODO: Remove these once vdp_rend_m5_x86.asm is fully ported to C++.
 */
extern "C" {
	void Render_Line_ScrollA(void);
	void Render_Line_ScrollA_Interlaced(void);
	void Render_Line_ScrollA_VScroll(void);
	void Render_Line_ScrollA_VScroll_Interlaced(void);
	void Render_Line_ScrollA_HS(void);
	void Render_Line_ScrollA_HS_Interlaced(void);
	void Render_Line_ScrollA_HS_VScroll(void);
	void Render_Line_ScrollA_HS_VScroll_Interlaced(void);
}
void Render_Line_ScrollA(void)
{ T_Render_Line_ScrollA<false, false, false>(); }
void Render_Line_ScrollA_Interlaced(void)
{ T_Render_Line_ScrollA<true, false, false>(); }
void Render_Line_ScrollA_VScroll(void)
{ T_Render_Line_ScrollA<false, true, false>(); }
void Render_Line_ScrollA_VScroll_Interlaced(void)
{ T_Render_Line_ScrollA<true, true, false>(); }
void Render_Line_ScrollA_HS(void)
{ T_Render_Line_ScrollA<false, false, true>(); }
void Render_Line_ScrollA_HS_Interlaced(void)
{ T_Render_Line_ScrollA<true, false, true>(); }
void Render_Line_ScrollA_HS_VScroll(void)
{ T_Render_Line_ScrollA<false, true, true>(); }
void Render_Line_ScrollA_HS_VScroll_Interlaced(void)
{ T_Render_Line_ScrollA<true, true, true>(); }


/**
 * T_Render_Line_Sprite(): Render a sprite line.
 * @param interlaced	[in] True for interlaced; false for non-interlaced.
 * @param h_s		[in] Highlight/Shadow enable.
 */
template<bool interlaced, bool h_s>
static inline void T_Render_Line_Sprite(void)
{
	// Update the sprite masks.
	unsigned int num_spr;
	if (Sprite_Over)
		num_spr = T_Update_Mask_Sprite<true, interlaced>();
	else
		num_spr = T_Update_Mask_Sprite<false, interlaced>();
	
	for (unsigned int spr_vis = 0; spr_vis < num_spr; spr_vis++)
	{
		// Get the sprite number. (TODO: Eliminate the sizeof division.)
		unsigned int spr_num = Sprite_Visible[spr_vis] / sizeof(Sprite_Struct[0]);
		
		// Determine the cell and line offsets.
		unsigned int cell_offset;
		unsigned int line_offset;
#ifdef FLICKERING_INTERLACED
		if (interlaced)
		{
			// Interlaced.
			cell_offset = ((VDP_Lines.Visible.Current * 2) - Sprite_Struct[spr_num].Pos_Y);
			if (VDP_Status & 0x0010)
				cell_offset++;
			
			line_offset = (cell_offset & 15);
			cell_offset &= 0x1F0;
		}
		else
#endif
		{
			// Not interlaced.
			cell_offset = (VDP_Lines.Visible.Current - Sprite_Struct[spr_num].Pos_Y);
			line_offset = (cell_offset & 7);
			cell_offset &= 0xF8;
		}
		
		// Get the Y cell size.
		unsigned int Y_cell_size = Sprite_Struct[spr_num].Size_Y;
		
		// Get the sprite information.
		// Also, check for swapped sprite layer priority.
		unsigned int spr_info = Sprite_Struct[spr_num].Num_Tile;
		if (VDP_Layers & VDP_LAYER_SPRITE_SWAP)
			spr_info ^= 0x8000;
		
		// Get the palette number, multiplied by 16.
		const unsigned int palette = ((spr_info >> 9) & 0x30);
		
		// Get the pattern number.
		unsigned int tile_num;
		if (interlaced)
		{
			tile_num = (spr_info & 0x3FF) << 6;	// point on the contents of the pattern
			
			Y_cell_size <<= 6;	// Size_Y * 64
#ifdef FLICKERING_INTERLACED
			cell_offset *= 4;	// Num_Pattern * 64
#else
			cell_offset *= 8;	// Num_Pattern * 64
#endif
		}
		else
		{
			tile_num = (spr_info & 0x7FF) << 5;	// point on the contents of the pattern
			
			Y_cell_size <<= 5;	// Size_Y * 32
			cell_offset *= 4;	// Num_Pattern * 32
		}
		
		// Check for V Flip.
		if (spr_info & 0x1000)
		{
			// V Flip enabled.
#ifdef FLICKERING_INTERLACED
			if (interlaced)
				line_offset ^= 15;
			else
#endif
				line_offset ^= 7;
			
			tile_num += (Y_cell_size - cell_offset);
			if (interlaced)
			{
				Y_cell_size += 64;
#ifdef FLICKERING_INTERLACED
				tile_num += (line_offset * 4);
#else
				tile_num += (line_offset * 8);
#endif
			}
			else
			{
				Y_cell_size += 32;
				tile_num += (line_offset * 4);
			}
		}
		else
		{
			// V Flip disabled.
			tile_num += cell_offset;
			if (interlaced)
			{
				Y_cell_size += 64;
#ifdef FLICKERING_INTERLACED
				tile_num += (line_offset * 4);
#else
				tile_num += (line_offset * 8);
#endif
			}
			else
			{
				Y_cell_size += 32;
				tile_num += (line_offset * 4);
			}
		}
		
		// Check for H Flip.
		int H_Pos_Min;
		int H_Pos_Max;
		
		if (spr_info & 0x800)
		{
			// H Flip enabled.
			// Check the minimum edge of the sprite.
			H_Pos_Min = Sprite_Struct[spr_num].Pos_X;
			if (H_Pos_Min < -7)
				H_Pos_Min = -7;	// minimum edge = clip screen
			
			H_Pos_Max = Sprite_Struct[spr_num].Pos_X_Max;
			
			H_Pos_Max -= 7;				// to post the last pattern in first
			while (H_Pos_Max >= VDP_Reg.H_Pix)
			{
				H_Pos_Max -= 8;			// move back to the preceding pattern (screen)
				tile_num += Y_cell_size;	// go to the next pattern (VRam)
			}
			
			// Draw the sprite.
			if ((VDP_Layers & VDP_LAYER_SPRITE_ALWAYSONTOP) || (spr_info & 0x8000))
			{
				// High priority.
				for (; H_Pos_Max >= H_Pos_Min; H_Pos_Max -= 8)
				{
					uint32_t pattern = VRam.u32[tile_num >> 2];
					T_PutLine_Sprite<true, h_s, true>(H_Pos_Max, pattern, palette);
					tile_num += Y_cell_size;
				}
			}
			else
			{
				// Low priority.
				for (; H_Pos_Max >= H_Pos_Min; H_Pos_Max -= 8)
				{
					uint32_t pattern = VRam.u32[tile_num >> 2];
					T_PutLine_Sprite<false, h_s, true>(H_Pos_Max, pattern, palette);
					tile_num += Y_cell_size;
				}
			}
		}
		else
		{
			// H Flip disabled.
			// Check the minimum edge of the sprite.
			H_Pos_Min = Sprite_Struct[spr_num].Pos_X;
			H_Pos_Max = Sprite_Struct[spr_num].Pos_X_Max;
			if (H_Pos_Max >= VDP_Reg.H_Pix)
				H_Pos_Max = VDP_Reg.H_Pix;
			
			while (H_Pos_Min < -7)
			{
				H_Pos_Min += 8;			// advance to the next pattern (screen)
				tile_num += Y_cell_size;	// go to the next pattern (VRam)
			}
			
			// Draw the sprite.
			if ((VDP_Layers & VDP_LAYER_SPRITE_ALWAYSONTOP) || (spr_info & 0x8000))
			{
				// High priority.
				for (; H_Pos_Min < H_Pos_Max; H_Pos_Min += 8)
				{
					uint32_t pattern = VRam.u32[tile_num >> 2];
					T_PutLine_Sprite<true, h_s, false>(H_Pos_Min, pattern, palette);
					tile_num += Y_cell_size;
				}
			}
			else
			{
				// Low priority.
				for (; H_Pos_Min < H_Pos_Max; H_Pos_Min += 8)
				{
					uint32_t pattern = VRam.u32[tile_num >> 2];
					T_PutLine_Sprite<false, h_s, false>(H_Pos_Min, pattern, palette);
					tile_num += Y_cell_size;
				}
			}
		}
	}
}

/**
 * C wrapper functions for T_Render_Line_Sprite().
 * TODO: Remove these once vdp_rend_m5_x86.asm is fully ported to C++.
 */
extern "C" {
	void Render_Line_Sprite(void);
	void Render_Line_Sprite_Interlaced(void);
	void Render_Line_Sprite_HS(void);
	void Render_Line_Sprite_HS_Interlaced(void);
}
void Render_Line_Sprite(void)
{ T_Render_Line_Sprite<false, false>(); }
void Render_Line_Sprite_Interlaced(void)
{ T_Render_Line_Sprite<true, false>(); }
void Render_Line_Sprite_HS(void)
{ T_Render_Line_Sprite<false, true>(); }
void Render_Line_Sprite_HS_Interlaced(void)
{ T_Render_Line_Sprite<true, true>(); }


/**
 * T_Render_LineBuf(): Render the line buffer to the destination surface.
 * @param pixel Type of pixel.
 * @param md_palette MD palette buffer.
 * @param dest Destination surface.
 */
template<typename pixel, pixel *md_palette>
static inline void T_Render_LineBuf(pixel *dest)
{
	const unsigned int num_px = (160 - VDP_Reg.H_Pix_Begin) * 2;
	const LineBuf_px_t *src = &LineBuf.px[8];
	
	// Draw the left border.
	// NOTE: S/H is ignored if we're in the border region.
	if (VDP_Reg.H_Pix_Begin != 0)
	{
		for (unsigned int i = (VDP_Reg.H_Pix_Begin / 4); i != 0; i--, dest += 4)
		{
			*dest     = md_palette[0];
			*(dest+1) = md_palette[0];
			*(dest+2) = md_palette[0];
			*(dest+3) = md_palette[0];
		}
	}
	
	// Render the line buffer to the destination surface.
	for (unsigned int i = (num_px / 4); i != 0; i--, dest += 4, src += 4)
	{
		// TODO: Endianness conversions.
		*dest     = md_palette[src->pixel];
		*(dest+1) = md_palette[(src+1)->pixel];
		*(dest+2) = md_palette[(src+2)->pixel];
		*(dest+3) = md_palette[(src+3)->pixel];
	}
	
	// Draw the right border.
	// NOTE: S/H is ignored if we're in the border region.
	if (VDP_Reg.H_Pix_Begin != 0)
	{
		for (unsigned int i = (VDP_Reg.H_Pix_Begin / 4); i != 0; i--, dest += 4)
		{
			*dest     = md_palette[0];
			*(dest+1) = md_palette[0];
			*(dest+2) = md_palette[0];
			*(dest+3) = md_palette[0];
		}
	}
}


/**
 * VDP_Render_Line_m5(): Render a line. (Mode 5)
 */
void VDP_Render_Line_m5(void)
{
	// Determine what part of the screen we're in.
	bool in_border = false;
	if (VDP_Lines.Visible.Current >= -VDP_Lines.Visible.Border_Size &&
	    VDP_Lines.Visible.Current < 0)
	{
		// Top border.
		in_border = true;
	}
	else if (VDP_Lines.Visible.Current >= VDP_Lines.Visible.Total &&
		 VDP_Lines.Visible.Current < (VDP_Lines.Visible.Total + VDP_Lines.Visible.Border_Size))
	{
		// Bottom border.
		in_border = true;
	}
	else if (VDP_Lines.Visible.Current < -VDP_Lines.Visible.Border_Size ||
		 VDP_Lines.Visible.Current >= (VDP_Lines.Visible.Total + VDP_Lines.Visible.Border_Size))
	{
		// Off screen.
		return;
	}
	
	// Determine the starting line in MD_Screen.
	int LineStart = VDP_Lines.Visible.Current;
	if ((CPU_Mode == 0) && (VDP_Reg.m5.Set2 & 0x08) && Video.ntscV30rolling)
	{
		// NTSC V30 mode. Simulate screen rolling.
		LineStart -= VDP_Lines.NTSC_V30.Offset;
		
		// Prevent underflow.
		if (LineStart < 0)
			LineStart += 240;
	}
	LineStart = TAB336[LineStart + VDP_Lines.Visible.Border_Size] + 8;
	
	if (in_border && !Video.borderColorEmulation)
	{
		// We're in the border area, but border color emulation is disabled.
		// Clear the border area.
		// TODO: Only clear this if the option changes or V/H mode changes.
		if (bppMD == 32)
			memset(&MD_Screen32[LineStart], 0x00, VDP_Reg.H_Pix*sizeof(uint32_t));
		else
			memset(&MD_Screen[LineStart], 0x00, VDP_Reg.H_Pix*sizeof(uint16_t));
		
		// ...and we're done here.
		return;
	}
	
	// Check if the VDP is enabled.
	if (!(VDP_Reg.m5.Set2 & 0x40) || in_border)
	{
		// VDP is disabled, or this is the border region.
		// Clear the line buffer.
		
		// NOTE: S/H is ignored if the VDP is disabled or if
		// we're in the border region.
		memset(LineBuf.u8, 0x00, sizeof(LineBuf.u8));
	}
	else
	{
		// VDP is enabled.
		
		// Check if sprite structures need to be updated.
		if (VDP_Reg.Interlaced)
		{
			// Interlaced.
			if (VDP_Flags.VRam)
				T_Make_Sprite_Struct<true, false>();
			else if (VDP_Flags.VRam_Spr)
				T_Make_Sprite_Struct<true, true>();
		}
		else
		{
			// Non-Interlaced.
			if (VDP_Flags.VRam)
				T_Make_Sprite_Struct<false, false>();
			else if (VDP_Flags.VRam_Spr)
				T_Make_Sprite_Struct<false, true>();
		}
		
		// Clear the VRam flags.
		VDP_Flags.VRam = 0;
		VDP_Flags.VRam_Spr = 0;
		
		// Run the rest of the asm code.
		VDP_Render_Line_m5_asm();
	}
	
	// Check if the palette was modified.
	if (VDP_Flags.CRam)
	{
		// Update the palette.
		if (VDP_Reg.m5.Set4 & 0x08)
			VDP_Update_Palette_HS();
		else
			VDP_Update_Palette();
	}
	
	// Render the image.
	if (bppMD != 32)
		T_Render_LineBuf<uint16_t, MD_Palette>(&MD_Screen[LineStart]);
	else
		T_Render_LineBuf<uint32_t, MD_Palette32>(&MD_Screen32[LineStart]);
}
