/***************************************************************************
 * Gens: Fast Blur function. (15/16/32-bit color, C version.)              *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
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


#include "fastblur.h"
#include "fastblur_16.h"
#include "misc.h"
#include "emulator/g_main.hpp"
#include "gens_core/vdp/vdp_rend.h"


#ifndef GENS_X86_ASM
static void Fast_Blur_16(void);
#endif
static void Fast_Blur_32(void);


/**
 * Fast_Blur: Apply a fast blurring algorithm to the onscreen image.
 */
void Fast_Blur(void)
{
	// TODO: Make it so fast blur doesn't apply to screenshots.
	if (bpp == 15 || bpp == 16)
	{
#ifdef GENS_X86_ASM
		if (Have_MMX)
			Fast_Blur_16_asm_MMX();
		else
			Fast_Blur_16_asm();
#else
		Fast_Blur_16();
#endif
	}
	else // if (bpp == 32)
		Fast_Blur_32();
}


#ifndef GENS_X86_ASM
static void Fast_Blur_16(void)
{
	int pixel;
	unsigned short color = 0, colorRB, colorG;
	unsigned short blurColorRB = 0, blurColorG = 0;
	
	unsigned short maskRB, maskG;
	
	// Check bpp.
	if (bpp == 15)
	{
		maskRB = 0x3C0F;
		maskG = 0x01E0;
	}
	else //if (bpp == 16)
	{
		maskRB = 0x780F;
		maskG = 0x03E0;
	}
	
	for (pixel = 1; pixel < (336 * 240); pixel++)
	{
		color = MD_Screen[pixel] >> 1;
		
		// Split the RB and G components.
		colorRB = color & maskRB;
		colorG = color & maskG;
		
		// Blur the colors with the previous pixels.
		blurColorRB += colorRB;
		blurColorG += colorG;
		
		// Draw the new pixel.
		MD_Screen[pixel - 1] = blurColorRB + blurColorG;
		
		// Save the components for the next pixel.
		blurColorRB = colorRB;
		blurColorG = colorG;
	}
}
#endif


static void Fast_Blur_32(void)
{
	int pixel;
	int color = 0, colorRB, colorG;
	int blurColorRB = 0, blurColorG = 0;
	
	for (pixel = 1; pixel < (336 * 240); pixel++)
	{
		color = MD_Screen32[pixel] >> 1;
		
		// Split the RB and G components.
		colorRB = color & 0x7F007F;
		colorG = color & 0x007F00;
		
		// Blur the colors with the previous pixels.
		blurColorRB += colorRB;
		blurColorG += colorG;
		
		// Draw the new pixel.
		MD_Screen32[pixel - 1] = blurColorRB + blurColorG;
		
		// Save the components for the next pixel.
		blurColorRB = colorRB;
		blurColorG = colorG;
	}
}
