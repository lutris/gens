/***************************************************************************
 * Gens: 2x renderer.                                                      *
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include "vdp_rend.h"
#include "blit.h"
#include "byteswap.h"
#include "misc.h"


extern unsigned char Bits32;


#ifndef GENS_X86_ASM
static void Blit_2x_16(unsigned char *Dest, int pitch, int x, int y, int offset);
#endif
static void Blit_2x_32(unsigned char *Dest, int pitch, int x, int y, int offset);


/**
 * Blit_2x(): Blits the image to the screen, double-sized.
 * @param Dest Destination buffer.
 * @param pitch Number of bytes per line.
 * @param x X coordinate for the image.
 * @param y Y coordinate for the image.
 * @param offset ???
 */
// TODO: Return a function pointer so this can be set in ui_proxy.c:Set_Render().
// That will reduce function call overhead.
void Blit_2x(unsigned char *Dest, int pitch, int x, int y, int offset)
{
	if (Bits32)
		Blit_2x_32(Dest, pitch, x, y, offset);
	else
	{
#ifdef GENS_X86_ASM
		if (Have_MMX)
			Blit_2x_16_asm_MMX(Dest, pitch, x, y, offset);
		else
			Blit_2x_16_asm(Dest, pitch, x, y, offset);
#else
		Blit_2x_16(Dest, pitch, x, y, offset);
#endif
	}
}


#ifndef GENS_X86_ASM
/**
 * Blit_2x_16(): (16-bit) Blits the image to the screen, double-sized.
 * @param Dest Destination buffer.
 * @param pitch Number of bytes per line.
 * @param x X coordinate for the image.
 * @param y Y coordinate for the image.
 * @param offset ???
 */
static void Blit_2x_16(unsigned char *Dest, int pitch, int x, int y, int offset)
{
	int i, j;
	
	int ScrWdth, ScrAdd;
	int SrcOffs, DstOffs;
	
	ScrAdd = offset >> 1;
	ScrWdth = x + ScrAdd;
	
	SrcOffs = 8;
	DstOffs = 0;
	
	for (i = 0; i < y; i++)
	{
		//DstOffs = i * pitch * 2;
		DstOffs = i * pitch * 2;
		for (j = 0; j < x; j++)
		{
			Dest[DstOffs + 0] = (unsigned char)(MD_Screen[SrcOffs] & 0xFF);
			Dest[DstOffs + 1] = (unsigned char)(MD_Screen[SrcOffs] >> 8);
			Dest[DstOffs + 2] = (unsigned char)(MD_Screen[SrcOffs] & 0xFF);
			Dest[DstOffs + 3] = (unsigned char)(MD_Screen[SrcOffs] >> 8);
			
			Dest[DstOffs + pitch + 0] = (unsigned char)(MD_Screen[SrcOffs] & 0xFF);
			Dest[DstOffs + pitch + 1] = (unsigned char)(MD_Screen[SrcOffs] >> 8);
			Dest[DstOffs + pitch + 2] = (unsigned char)(MD_Screen[SrcOffs] & 0xFF);
			Dest[DstOffs + pitch + 3] = (unsigned char)(MD_Screen[SrcOffs] >> 8);
			
			SrcOffs++;
			DstOffs += 4;
		}
		
		// Next line.
		SrcOffs += ScrAdd;
	}
}
#endif


/**
 * Blit_2x_32(): (32-bit) Blits the image to the screen, double-sized.
 * @param Dest Destination buffer.
 * @param pitch Number of bytes per line.
 * @param x X coordinate for the image.
 * @param y Y coordinate for the image.
 * @param offset ???
 */
static void Blit_2x_32(unsigned char *Dest, int pitch, int x, int y, int offset)
{
	int i, j;
	
	int ScrWdth, ScrAdd;
	int SrcOffs, DstOffs;
	
	ScrAdd = offset >> 1;
	ScrWdth = x + ScrAdd;
	
	SrcOffs = 8;
	DstOffs = 0;
	
	for (i = 0; i < y; i++)
	{
		//DstOffs = i * pitch * 2;
		DstOffs = i * pitch * 2;
		for (j = 0; j < x; j++)
		{
			cpu_to_le32_ucptr(&Dest[DstOffs + 0], MD_Screen32[SrcOffs]);
			cpu_to_le32_ucptr(&Dest[DstOffs + 4], MD_Screen32[SrcOffs]);
			cpu_to_le32_ucptr(&Dest[DstOffs + pitch + 0], MD_Screen32[SrcOffs]);
			cpu_to_le32_ucptr(&Dest[DstOffs + pitch + 4], MD_Screen32[SrcOffs]);
			
			SrcOffs++;
			DstOffs += 8;
		}
		
		// Next line.
		SrcOffs += ScrAdd;
	}
}
