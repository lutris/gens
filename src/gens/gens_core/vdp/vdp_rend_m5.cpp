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
 * VDP_Render_Line_m5(): Render a line. (Mode 5)
 */
void VDP_Render_Line_m5(void)
{
	unsigned int i;
	
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
	const register unsigned int num_2px = (160 - H_Pix_Begin) >> 1;
	const unsigned int LineStart = (TAB336[VDP_Current_Line] + 8);
		
	// Line Buffer is accessed using bytes for some reason.
	uint8_t *src = &LineBuf.u8[8<<1];
	
	if (bppMD == 32)
	{
		// 32-bit color.
		uint32_t *dest = &MD_Screen32[LineStart];
		for (i = num_2px; i != 0; i--)
		{
			// TODO: Endianness conversions.
			*dest     = MD_Palette32[*src];
			*(dest+1) = MD_Palette32[*(src+2)];
			*(dest+2) = MD_Palette32[*(src+4)];
			*(dest+3) = MD_Palette32[*(src+6)];
			
			dest += 4;
			src += 8;
		}
	}
	else
	{
		// 15/16-bit color.
		uint16_t *dest = &MD_Screen[LineStart];
		for (i = num_2px; i != 0; i--)
		{
			// TODO: Endianness conversions.
			*dest     = MD_Palette[*src];
			*(dest+1) = MD_Palette[*(src+2)];
			*(dest+2) = MD_Palette[*(src+4)];
			*(dest+3) = MD_Palette[*(src+6)];
			
			dest += 4;
			src += 8;
		}
	}
}
