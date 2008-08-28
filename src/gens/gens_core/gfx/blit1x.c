/***************************************************************************
 * Gens: 1x renderer.                                                      *
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
static void Blit_1x_16(unsigned char *screen, int pitch, int x, int y, int offset);
#endif
static void Blit_1x_32(unsigned char *screen, int pitch, int x, int y, int offset);


/**
 * Blit_1x(): Blits the image to the screen, with no scaling.
 * @param screen Screen buffer.
 * @param pitch Number of bytes per line.
 * @param x X coordinate for the image.
 * @param y Y coordinate for the image.
 * @param offset ???
 */
// TODO: Return a function pointer so this can be set in ui_proxy.c:Set_Render().
// That will reduce function call overhead.
void Blit_1x(unsigned char *screen, int pitch, int x, int y, int offset)
{
	if (Bits32)
		Blit_1x_32(screen, pitch, x, y, offset);
	else
	{
#ifdef GENS_X86_ASM
		if (Have_MMX)
			Blit_1x_16_asm_MMX(screen, pitch, x, y, offset);
		else
			Blit_1x_16_asm(screen, pitch, x, y, offset);
#else
		Blit_1x_16(screen, pitch, x, y, offset);
#endif
	}
}


#ifndef GENS_X86_ASM
/**
 * Blit_1x_16(): (16-bit) Blits the image to the screen, with no scaling.
 * @param screen Screen buffer.
 * @param pitch Number of bytes per line.
 * @param x X coordinate for the image.
 * @param y Y coordinate for the image.
 * @param offset ???
 */
static void Blit_1x_16(unsigned char *screen, int pitch, int x, int y, int offset)
{
	int i;
	
	int ScrWdth, ScrAdd;
	int SrcOffs, DstOffs;
	
	ScrAdd = offset >> 1;
	ScrWdth = x + ScrAdd;
	
	SrcOffs = 8;
	DstOffs = 0;
	
	for (i = 0; i < y; i++)
	{
		//DstOffs = i * pitch * 2;
		DstOffs = i * pitch;
		memcpy(&screen[DstOffs], &MD_Screen[SrcOffs], x * 2);
		
		// Next line.
		// TODO: Make this a constant somewhere.
		SrcOffs += 336;
	}
}
#endif


/**
 * Blit_1x_32(): (32-bit) Blits the image to the screen, with no scaling.
 * @param screen Screen buffer.
 * @param pitch Number of bytes per line.
 * @param x X coordinate for the image.
 * @param y Y coordinate for the image.
 * @param offset ???
 */
static void Blit_1x_32(unsigned char *screen, int pitch, int x, int y, int offset)
{
	int i;
	
	int ScrWdth, ScrAdd;
	int SrcOffs, DstOffs;
	
	ScrAdd = offset >> 1;
	ScrWdth = x + ScrAdd;
	
	SrcOffs = 8;
	DstOffs = 0;
	
	for (i = 0; i < y; i++)
	{
		//DstOffs = i * pitch * 2;
		DstOffs = i * pitch;
		memcpy(&screen[DstOffs], &MD_Screen32[SrcOffs], x * 4);
		
		// Next line.
		// TODO: Make this a constant somewhere.
		SrcOffs += 336;
	}
}
