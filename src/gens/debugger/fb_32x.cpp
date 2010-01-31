/***************************************************************************
 * Gens: 32X VDP framebuffer debugger.                                     *
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

#include "fb_32x.hpp"
#include "macros/force_inline.h"

#include "emulator/g_main.hpp"

#include "gens_core/vdp/vdp_io.h"
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/vdp/vdp_32x.h"
#include "gens_core/vdp/TAB336.h"

// C includes.
#include <stdint.h>
#include <stdio.h>


template<typename pixel>
static FORCE_INLINE void T_VDP_32X_Draw_FB(int fb_num, pixel *screen,
					   pixel *_32X_palette, pixel *_32X_vdp_cram_adjusted)
{
	// Determine the framebuffer to use.
	const unsigned int FB_Address = ((fb_num & 1) << 16);
	
	// Destination pixel.
	// NOTE: Debugger always uses 320x224.
	pixel *dest = &screen[(336*8)+8];
	
	// Temporary pixels.
	register pixel px1, px2;
	
	switch (_32X_VDP.Mode & 3)
	{
		case 0:
			// _32X_Draw_M00
			// DO NOTHING!
			return;
		
		case 1:
			// _32X_Draw_M01: 256-color.
			// TODO: Endianness conversions.
			for (unsigned int y = 0; y < 224; y++)
			{
				const unsigned int offset = _32X_VDP_Ram.u16[FB_Address + y];
				uint8_t *src = &_32X_VDP_Ram.u8[(FB_Address + offset) << 1];
				
				for (unsigned int x = (320/2); x != 0; x--, src += 2, dest += 2)
				{
					px1 = _32X_vdp_cram_adjusted[*src];
					px2 = _32X_vdp_cram_adjusted[*(src+1)];
					
					// NOTE: Destination pixels are swapped in this mode.
					*dest = px2;
					*(dest+1) = px1;
				}
				
				// Next line.
				dest += 16;
			}
			break;
		
		case 2:
			// _32X_Draw_M10: 15-bit color.
			// TODO: Test this!
			for (unsigned int y = 0; y < 224; y++)
			{
				const unsigned int offset = _32X_VDP_Ram.u16[FB_Address + y];
				uint16_t *src = &_32X_VDP_Ram.u16[FB_Address + offset];
				
				for (unsigned int x = (320/2); x != 0; x--, src += 2, dest += 2)
				{
					px1 = _32X_palette[*src];
					px2 = _32X_palette[*(src+1)];
					
					*dest = px1;
					*(dest+1) = px2;
				}
				
				// Next line.
				dest += 16;
			}
			break;
		
		case 3:
			// _32X_Draw_M11: 256-color RLE.
			// TODO: Port this!
			// TODO: Endianness conversions.
			break;
		
		default:
			// to make gcc shut up
			break;
	}
	
	// Draw the palette.
	dest = &screen[(336*(8+224))+8];
	for (unsigned int i = (256/2); i != 0; i--, dest += 2, _32X_vdp_cram_adjusted += 2)
	{
		px1 = *_32X_vdp_cram_adjusted;
		px2 = *(_32X_vdp_cram_adjusted+1);
		
		// Line 1.
		*dest = px1;
		*(dest+1) = px2;
		
		// Line 2.
		*(dest+336) = px1;
		*(dest+336+1) = px2;
	}
}


void VDP_32X_Draw_FB(int fb_num)
{
	if (bppMD != 32)
		T_VDP_32X_Draw_FB<uint16_t>(fb_num, MD_Screen.u16, _32X_Palette.u16, _32X_VDP_CRam_Adjusted.u16);
	else
		T_VDP_32X_Draw_FB<uint32_t>(fb_num, MD_Screen.u32, _32X_Palette.u32, _32X_VDP_CRam_Adjusted.u32);
}
