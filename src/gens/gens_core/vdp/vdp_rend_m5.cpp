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
			// Shadow/Highlight is enabled. Clear with 0x40.
			memset(LineBuf.u8, 0x40, sizeof(LineBuf.u8));
		}
		else
		{
			// Shadow/Highlight is disabled. Clear with 0x00.
			memset(LineBuf.u8, 0x00, sizeof(LineBuf.u8));
		}
	}
	else
	{
		// VDP is active.
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
