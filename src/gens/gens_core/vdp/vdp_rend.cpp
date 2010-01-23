/***************************************************************************
 * Gens: VDP rendering functions.                                          *
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

#include "vdp_rend.h"
#include "vdp_io.h"

// VDP rendering includes.
#include "vdp_rend_err.hpp"	// Error message drawing.
#include "vdp_rend_m5.hpp"	// Mode 5 renderer.

// bppMD
#include "emulator/g_main.hpp"

// C includes.
#include <stdint.h>

// Full MD palette.
Palette_t Palette;

// VDP layer control.
// TODO: Figure out 32X and Mode 4 layer assignments.
unsigned int VDP_Layers = VDP_LAYER_DEFAULT;

// MD screen and palette buffers.
// TODO: Unionize these!
uint16_t MD_Screen[336*240];
uint32_t MD_Screen32[336*240];
uint16_t MD_Palette[0x100];
uint32_t MD_Palette32[0x100];

// Sprite structures.
// TODO: Move these to vdp_rend_m5.cpp.
Sprite_Struct_t Sprite_Struct[0x100];
int Sprite_Visible[0x100];

// Sprite Limit toggle.
int Sprite_Over;


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
			
			// Highlight color. (1xxx - 0001)
			MD_palette[i + 128]	= palette[(0x888 | color1_raw) - 0x111];
			MD_palette[i + 1 + 128]	= palette[(0x888 | color2_raw) - 0x111];
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
	if (bppMD != 32)
		T_VDP_Update_Palette<false>(MD_Palette, Palette.u16);
	else
		T_VDP_Update_Palette<false>(MD_Palette32, Palette.u32);
}

/**
 * VDP_Update_Palette_HS(): Update the palette, including highlight and shadow.
 */
void VDP_Update_Palette_HS(void)
{
	if (bppMD != 32)
		T_VDP_Update_Palette<true>(MD_Palette, Palette.u16);
	else
		T_VDP_Update_Palette<true>(MD_Palette32, Palette.u32);
}


/**
 * VDP_Render_Line(): Render a line.
 */
void VDP_Render_Line(void)
{
	// TODO: 32X-specific function.
	if (VDP_Mode & VDP_MODE_M5)
	{
		// Mode 5.
		if (_32X_Started)
			VDP_Render_Line_m5_32X();
		else
			VDP_Render_Line_m5();
	}
	else
	{
		// Unsupported mode.
		VDP_Render_Error();
	}
	
	// Update the VDP render error cache.
	VDP_Render_Error_Update();
}
