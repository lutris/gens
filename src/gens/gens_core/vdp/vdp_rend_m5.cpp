/***************************************************************************
 * Gens: VDP Renderer. (Mode 5)                                            *
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

#include "vdp_rend_m5.hpp"
#include "vdp_rend.h"
#include "vdp_io.h"

#include "emulator/g_main.hpp"

// C includes.
#include <stdint.h>
#include <string.h>


// Line buffer for current line.
// TODO: Mark as static once VDP_Render_Line_m5_asm is ported to C.
typedef union
{
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
	uint16_t *CurSpr = (uint16_t*)Spr_Addr;
	unsigned int spr_num = 0;
	unsigned int link;
	
	// H40 allows 80 sprites; H32 allows 64 sprites.
	// TODO: Test 0x81 instead of 0x01?
	const unsigned int max_spr = (VDP_Reg.Set4 & 0x01) ? 80 : 64;
	
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
				
				// Interlaced: Y-pos is divided by 2.
				Sprite_Struct[spr_num].Pos_Y = ((*CurSpr & 0x3FF) / 2) - 128;
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
			Sprite_Struct[spr_num].Pos_Y_Max =
					Sprite_Struct[spr_num].Pos_Y +
					((Sprite_Struct[spr_num].Size_Y * 8) + 7);
			
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
		CurSpr = ((uint16_t*)Spr_Addr) + (link * (8>>1));
		
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
 * T_Update_Mask_Sprite(): Update Sprite_Visible using sprite masking.
 * @param sprite_limit If true, emulates sprite limits.
 * @return Last sprite visible * sizeof(Sprite_Visible[0]).
 */
template<bool sprite_limit>
static inline unsigned int T_Update_Mask_Sprite(void)
{
	// If Sprite Limit is on, the following limits are enforced: (H32/H40)
	// - Maximum sprite dots per line: 256/320
	// - Maximum sprites per line: 16/20
	int max_cells = H_Cell;
	int max_sprites = (H_Cell / 2);
	
	bool overflow = false;
	
	// Sprite masking variables.
	bool sprite_on_line = false;	// True if at least one sprite is on the scanline.
	
	unsigned int spr_num = 0;	// Current sprite number in Sprite_Struct[].
	unsigned int spr_vis = 0;	// Current visible sprite in Sprite_Visible[].
	
	// Number of sprites in Sprite_Struct.
	const unsigned int TotalSprites = (VDP_Data_Misc.Spr_End / sizeof(Sprite_Struct_t)) + 1;
	
	// Search for all sprites visible on the current scanline.
	for (; spr_num < TotalSprites; spr_num++)
	{
		// Check if the sprite is on the current line.
		if (Sprite_Struct[spr_num].Pos_Y > VDP_Current_Line ||
		    Sprite_Struct[spr_num].Pos_Y_Max < VDP_Current_Line)
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
		if (Sprite_Struct[spr_num].Pos_X < H_Pix &&
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
			if (Sprite_Struct[spr_num].Pos_Y > VDP_Current_Line ||
			    Sprite_Struct[spr_num].Pos_Y_Max < VDP_Current_Line)
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
	
	// Save the number of visible sprites.
	VDP_Data_Misc.Borne = (spr_vis * sizeof(Sprite_Visible[0]));
	return VDP_Data_Misc.Borne;
}


/**
 * C wrapper functions for T_Update_Mask_Sprite().
 * TODO: Remove these once vdp_rend_m5_x86.asm is fully ported to C++.
 */
extern "C" {
	unsigned int Update_Mask_Sprite(void);
	unsigned int Update_Mask_Sprite_Limit(void);
}

unsigned int Update_Mask_Sprite(void)
{
	return T_Update_Mask_Sprite<false>();
}
unsigned int Update_Mask_Sprite_Limit(void)
{
	return T_Update_Mask_Sprite<true>();
}


/**
 * T_Get_X_Offset(): Get the X offset for the line. (Horizontal Scroll Table)
 * @param plane True for Scroll A; false for Scroll B.
 * @return X offset.
 */
template<bool plane>
static inline uint16_t T_Get_X_Offset(void)
{
	const unsigned int H_Scroll_Offset = (VDP_Current_Line & H_Scroll_Mask) * 2;
	
	if (plane)
	{
		// Scroll A.
		return H_Scroll_Addr[H_Scroll_Offset];
	}
	else
	{
		// Scroll B.
		return H_Scroll_Addr[H_Scroll_Offset + 1];
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
 * @param cur Current Y offset. (Returned if we're outside of VRam limits.)
 * @return Y offset.
 */
template<bool plane, bool interlaced>
static inline unsigned int T_Update_Y_Offset(unsigned int cur)
{
	// TODO: This function is untested!
	// Find a ROM that uses 2-cell VScroll to test it.
	
	if (VDP_Data_Misc.Cell & 0xFF81)
	{
		// Outside of VRam limits. Don't change anything.
		return cur;
	}
	
	// Get the vertical scroll offset.
	unsigned int VScroll_Offset = VDP_Data_Misc.Cell;
	if (plane)
	{
		// Scroll A.
		VScroll_Offset = VSRam.u16[VScroll_Offset];
	}
	else
	{
		// Scroll B.
		VScroll_Offset = VSRam.u16[VScroll_Offset + 1];
	}
	
	if (interlaced)
	{
		// Divide Y scroll by 2.
		// TODO: Don't do this! Handle interlaced mode properly.
		VScroll_Offset /= 2;
	}
	
	VScroll_Offset += VDP_Current_Line;
	VDP_Data_Misc.Line_7 = VScroll_Offset & 0x07;	// Pattern line.
	VScroll_Offset >>= 3;				// Get the V Cell offset.
	VScroll_Offset &= V_Scroll_CMask;		// Prevent V Cell offset from overflowing.
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
{
	return T_Update_Y_Offset<true, false>(cur);
}
unsigned int Update_Y_Offset_ScrollB(unsigned int cur)
{
	return T_Update_Y_Offset<false, false>(cur);
}
unsigned int Update_Y_Offset_ScrollA_Interlaced(unsigned int cur)
{
	return T_Update_Y_Offset<true, true>(cur);
}
unsigned int Update_Y_Offset_ScrollB_Interlaced(unsigned int cur)
{
	return T_Update_Y_Offset<false, true>(cur);
}


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
	unsigned int offset = (y << H_Scroll_CMul) + x;
	
	// Return the pattern information.
	return (plane ? ScrA_Addr[offset] : ScrB_Addr[offset]);
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
 * T_Get_Pattern_Data(): Get pattern data for a given tile for the current line.
 * @param interlaced True for interlaced; false for non-interlaced.
 * @param pattern Pattern info.
 * @return Pattern data.
 */
template<bool interlaced>
static inline unsigned int T_Get_Pattern_Data(uint16_t pattern)
{
	unsigned int V_Offset = VDP_Data_Misc.Line_7;	// Vertical offset.
	unsigned int TileAddr = (pattern & 0x7FF);	// Tile number.
	
	// Get the tile address.
	if (interlaced)
		TileAddr <<= 6;
	else
		TileAddr <<= 5;
	
	if (pattern & 0x1000)
	{
		// V Flip enabled. Flip the tile vertically.
		// TODO: Proper interlace support requires 8x16 cells.
		V_Offset ^= 7;
	}
	
	if (interlaced)
		return VRam.u32[(TileAddr + (V_Offset * 8)) >> 2];
	else
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

#define LINEBUF_HIGH_W	0x8080
#define LINEBUF_SHAD_W	0x4040
#define LINEBUF_PRIO_W	0x0100
#define LINEBUF_SPR_W	0x2000

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
	const unsigned int LineBuf_pixnum = ((disp_pixnum + pat_pixnum) * 2);
	
	// TODO: Endianness conversions.
	uint8_t layer_bits = LineBuf.u8[LineBuf_pixnum + 1];
	if (plane && (layer_bits & LINEBUF_PRIO_B))
	{
		// Scroll A; however, pixel has priority set.
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
	LineBuf.u8[LineBuf_pixnum] = pat8;
}


/**
 * T_PutPixel_P1(): Put a pixel in background graphics layer 1.
 * @param h_s		[in] Highlight/Shadow enable.
 * @param pat_pixnum	[in] Pattern pixel number.
 * @param mask		[in] Mask to isolate the good pixel.
 * @param shift		[in] Shift.
 * @param disp_pixnum	[in] Display pixel number.
 * @param pattern	[in] Pattern data.
 * @param palette	[in] Palette number * 16.
 */
template<bool h_s, int pat_pixnum, uint32_t mask, int shift>
static inline void T_PutPixel_P1(int disp_pixnum, uint32_t pattern, unsigned int palette)
{
	// TODO: Convert mask and shift to template parameters.
	
	// Check if this is a transparent pixel.
	unsigned int px = (pattern & mask);
	if (px == 0)
		return;
	
	// Shift the pixel.
	px >>= shift;
	
	// Update the pixel:
	// - Add palette information.
	// - Mark the pixel as priority.
	// - Save it to the linebuffer.
	px |= palette | LINEBUF_PRIO_W;
	
	const unsigned int LineBuf_pixnum = (disp_pixnum + pat_pixnum);
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
	const unsigned int LineBuf_pixnum = ((disp_pixnum + pat_pixnum + 8) * 2);
	
	// TODO: Endianness conversions.
	uint8_t layer_bits = LineBuf.u8[LineBuf_pixnum + 1];
	
	if (layer_bits & (LINEBUF_PRIO_B + LINEBUF_SPR_B - priority))
	{
		// Priority bit is set. (TODO: Is that what this means?)
		if (!priority)
		{
			// Set the sprite bit in the linebuffer.
			LineBuf.u8[LineBuf_pixnum + 1] |= LINEBUF_SPR_B;
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
			LineBuf.u16[LineBuf_pixnum>>1] |= LINEBUF_HIGH_W;
			return 0;
		}
		else if (px == 0x3F)
		{
			// Palette 3, color 15: Shadow. (Sprite pixel doesn't show up.)
			LineBuf.u16[LineBuf_pixnum>>1] |= LINEBUF_SHAD_W;
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
	LineBuf.u16[LineBuf_pixnum>>1] = px;
	
	return 0;
}


#define LINEBUF_HIGH_D	0x80808080
#define LINEBUF_SHAD_D	0x40404040
#define LINEBUF_PRIO_D	0x01000100
#define LINEBUF_SPR_D	0x20002000

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
		T_PutPixel_P1<h_s, 0, 0x0000F000, 12>(disp_pixnum, pattern, palette);
		T_PutPixel_P1<h_s, 1, 0x00000F00,  8>(disp_pixnum, pattern, palette);
		T_PutPixel_P1<h_s, 2, 0x000000F0,  4>(disp_pixnum, pattern, palette);
		T_PutPixel_P1<h_s, 3, 0x0000000F,  0>(disp_pixnum, pattern, palette);
		T_PutPixel_P1<h_s, 4, 0xF0000000, 28>(disp_pixnum, pattern, palette);
		T_PutPixel_P1<h_s, 5, 0x0F000000, 24>(disp_pixnum, pattern, palette);
		T_PutPixel_P1<h_s, 6, 0x00F00000, 20>(disp_pixnum, pattern, palette);
		T_PutPixel_P1<h_s, 7, 0x000F0000, 16>(disp_pixnum, pattern, palette);
	}
	else
	{
		// Horizontal flip.
		T_PutPixel_P1<h_s, 0, 0x000F0000, 16>(disp_pixnum, pattern, palette);
		T_PutPixel_P1<h_s, 1, 0x00F00000, 20>(disp_pixnum, pattern, palette);
		T_PutPixel_P1<h_s, 2, 0x0F000000, 24>(disp_pixnum, pattern, palette);
		T_PutPixel_P1<h_s, 3, 0xF0000000, 28>(disp_pixnum, pattern, palette);
		T_PutPixel_P1<h_s, 4, 0x0000000F,  0>(disp_pixnum, pattern, palette);
		T_PutPixel_P1<h_s, 5, 0x000000F0,  4>(disp_pixnum, pattern, palette);
		T_PutPixel_P1<h_s, 6, 0x00000F00,  8>(disp_pixnum, pattern, palette);
		T_PutPixel_P1<h_s, 7, 0x0000F000, 12>(disp_pixnum, pattern, palette);
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
 * C wrapper functions for T_PutLine_Sprite().
 * TODO: Remove these once vdp_rend_m5_x86.asm is fully ported to C++.
 */
extern "C" {
	void PutLine_Sprite_High(int disp_pixnum, uint32_t pattern, int palette);
	void PutLine_Sprite_High_HS(int disp_pixnum, uint32_t pattern, int palette);
	void PutLine_Sprite_Low(int disp_pixnum, uint32_t pattern, int palette);
	void PutLine_Sprite_Low_HS(int disp_pixnum, uint32_t pattern, int palette);
	void PutLine_Sprite_Flip_High(int disp_pixnum, uint32_t pattern, int palette);
	void PutLine_Sprite_Flip_High_HS(int disp_pixnum, uint32_t pattern, int palette);
	void PutLine_Sprite_Flip_Low(int disp_pixnum, uint32_t pattern, int palette);
	void PutLine_Sprite_Flip_Low_HS(int disp_pixnum, uint32_t pattern, int palette);
}

void PutLine_Sprite_High(int disp_pixnum, uint32_t pattern, int palette)
{
	T_PutLine_Sprite<true, false, false>(disp_pixnum, pattern, palette);
}
void PutLine_Sprite_High_HS(int disp_pixnum, uint32_t pattern, int palette)
{
	T_PutLine_Sprite<true, true, false>(disp_pixnum, pattern, palette);
}
void PutLine_Sprite_Low(int disp_pixnum, uint32_t pattern, int palette)
{
	T_PutLine_Sprite<false, false, false>(disp_pixnum, pattern, palette);
}
void PutLine_Sprite_Low_HS(int disp_pixnum, uint32_t pattern, int palette)
{
	T_PutLine_Sprite<false, true, false>(disp_pixnum, pattern, palette);
}
void PutLine_Sprite_Flip_High(int disp_pixnum, uint32_t pattern, int palette)
{
	T_PutLine_Sprite<true, false, true>(disp_pixnum, pattern, palette);
}
void PutLine_Sprite_Flip_High_HS(int disp_pixnum, uint32_t pattern, int palette)
{
	T_PutLine_Sprite<true, true, true>(disp_pixnum, pattern, palette);
}
void PutLine_Sprite_Flip_Low(int disp_pixnum, uint32_t pattern, int palette)
{
	T_PutLine_Sprite<false, false, true>(disp_pixnum, pattern, palette);
}
void PutLine_Sprite_Flip_Low_HS(int disp_pixnum, uint32_t pattern, int palette)
{
	T_PutLine_Sprite<false, true, true>(disp_pixnum, pattern, palette);
}


/**
 * T_Render_Line_ScrollB(): Render a line for Scroll B.
 * @param interlaced	[in] True for interlaced; false for non-interlaced.
 * @param vscroll	[in] True for 2-cell mode; false for full scroll.
 * @param h_s		[in] Highlight/Shadow enable.
 */
template<bool interlaced, bool vscroll, bool h_s>
static inline void T_Render_Line_ScrollB(void)
{
	// Get the ScrollB offset. (cell and fine offset)
	unsigned int X_offset_cell = T_Get_X_Offset<false>() & 0x3FF;
	unsigned int X_offset_fine = X_offset_cell & 7;	// Fine offset.
	
	// Get the correct cell offset:
	// - Invert the cell position.
	// - Right-shift by 3 for the cell number.
	// - AND with the horizontal scrolling cell mask to prevent overflow.
	X_offset_cell = (((X_offset_cell ^ 0x3FF) >> 3) & H_Scroll_CMask);
	
	VDP_Data_Misc.X = H_Cell;			// Number of cells to draw.
	VDP_Data_Misc.Cell = (X_offset_cell & 1) - 2;	// Current cell for VScroll.
	
	// Drawing will start at LineBuf.u16[offset_fine].
	unsigned int disp_pixnum = X_offset_fine;
	
	// Initialize the Y offset.
	unsigned int Y_offset_cell = VSRam.u16[1];	// Index 1 is the first VSRam entry for Scroll B.
	
	if (interlaced)
	{
		// Divide Y scroll by 2.
		// TODO: Don't do this! Handle interlaced mode properly.
		Y_offset_cell /= 2;
	}
	
	Y_offset_cell += VDP_Current_Line;
	VDP_Data_Misc.Line_7 = (Y_offset_cell & 7);
	Y_offset_cell = (Y_offset_cell >> 3) & V_Scroll_CMask;
	goto Start_Loop;
	
	// Loop through the cells.
	for (; VDP_Data_Misc.X >= 0; VDP_Data_Misc.X--)
	{
		if (vscroll)
		{
			// 2-cell vertical scrolling.
			// Update the Y offset.
			Y_offset_cell = T_Update_Y_Offset<false, interlaced>(Y_offset_cell);
		}
		
Start_Loop:
		// Get pattern info and data for the current tile.
		uint32_t pattern_info = T_Get_Pattern_Info<false>(X_offset_cell, Y_offset_cell);
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
				T_PutLine_P1<false, h_s, true>(disp_pixnum, pattern_data, palette);
			else
				T_PutLine_P0<false, h_s, true>(disp_pixnum, pattern_data, palette);
		}
		else
		{
			// Pattern doesn't have flip enabled.
			if (pattern_info & 0x8000)
				T_PutLine_P1<false, h_s, false>(disp_pixnum, pattern_data, palette);
			else
				T_PutLine_P0<false, h_s, false>(disp_pixnum, pattern_data, palette);
		}
		
		// Go to the next H cell.
		VDP_Data_Misc.Cell++;
		X_offset_cell = (X_offset_cell + 1) & H_Scroll_CMask;
		
		// Go to the next pattern.
		disp_pixnum += 8;
	}
}

/**
 * C wrapper functions for T_PutLine_Sprite().
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
{ T_Render_Line_ScrollB<false, false, false>(); }
void Render_Line_ScrollB_Interlaced(void)
{ T_Render_Line_ScrollB<true, false, false>(); }
void Render_Line_ScrollB_VScroll(void)
{ T_Render_Line_ScrollB<false, true, false>(); }
void Render_Line_ScrollB_VScroll_Interlaced(void)
{ T_Render_Line_ScrollB<true, true, false>(); }
void Render_Line_ScrollB_HS(void)
{ T_Render_Line_ScrollB<false, false, true>(); }
void Render_Line_ScrollB_HS_Interlaced(void)
{ T_Render_Line_ScrollB<true, false, true>(); }
void Render_Line_ScrollB_HS_VScroll(void)
{ T_Render_Line_ScrollB<false, true, true>(); }
void Render_Line_ScrollB_HS_VScroll_Interlaced(void)
{ T_Render_Line_ScrollB<true, true, true>(); }


/**
 * T_Render_LineBuf(): Render the line buffer to the destination surface.
 * @param pixel Type of pixel.
 * @param md_palette MD palette buffer.
 * @param num_px Number of pixels to render.
 * @param src Source. (Line buffer)
 * @param dest Destination surface.
 */
template<typename pixel, pixel *md_palette>
static inline void T_Render_LineBuf(unsigned int num_px, uint8_t *src, pixel *dest)
{
	// Render the line buffer to the destination surface.
	// Line buffer is accessed using bytes for some reason.
	for (unsigned int i = (num_px / 4); i != 0; i--)
	{
		// TODO: Endianness conversions.
		*dest     = md_palette[*src];
		*(dest+1) = md_palette[*(src+2)];
		*(dest+2) = md_palette[*(src+4)];
		*(dest+3) = md_palette[*(src+6)];
			
		dest += 4;
		src += 8;
	}
}


/**
 * VDP_Render_Line_m5(): Render a line. (Mode 5)
 */
void VDP_Render_Line_m5(void)
{
	// Check if the VDP is active.
	if (!(VDP_Reg.Set2 & 0x40))
	{
		// VDP isn't active. Clear the line buffer.
		if (VDP_Reg.Set4 & 0x08)
		{
			// Highlight/Shadow is enabled. Clear with 0x40.
			memset(LineBuf.u8, 0x40, sizeof(LineBuf.u8));
		}
		else
		{
			// Highlight/Shadow is disabled. Clear with 0x00.
			memset(LineBuf.u8, 0x00, sizeof(LineBuf.u8));
		}
	}
	else
	{
		// VDP is active.
		
		// Check if sprite structures need to be updated.
		if (VDP_Reg.Set4 & 0x04)
		{
			// Interlaced.
			// TODO: This checks LSM1 only. Check both LSM1 and LSM0!
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
		if (VDP_Reg.Set4 & 0x08)
			VDP_Update_Palette_HS();
		else
			VDP_Update_Palette();
	}
	
	// Render the image.
	const unsigned int num_px = (160 - H_Pix_Begin) * 2;
	const unsigned int LineStart = (TAB336[VDP_Current_Line] + 8);
	
	if (bppMD == 32)
	{
		T_Render_LineBuf<uint32_t, MD_Palette32>
				(num_px, &LineBuf.u8[8<<1], &MD_Screen32[LineStart]);
	}
	else
	{
		T_Render_LineBuf<uint16_t, MD_Palette>
				(num_px, &LineBuf.u8[8<<1], &MD_Screen[LineStart]);
	}
}
