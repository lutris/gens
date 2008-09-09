/***************************************************************************
 * Gens: Scale2x renderer: http://scale2x.sourceforge.net/                 *
 *                                                                         *
 * Copyright (c) 2001 by Andrea Mazzoleni                                  *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
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

#include "gens_core/vdp/vdp_rend.h"
#include "blit.h"

/**
 * Blit_Scale2x: Blits the image to the screen, 2x size, Scale2x filter.
 * @param screen Pointer to the screen buffer.
 * @param mdScreen Pointer to the MD screen buffer.
 * @param pitch Number of bytes per line.
 * @param x X coordinate for the image.
 * @param y Y coordinate for the image.
 * @param offset ???
 */
template<typename pixel>
static inline void Blit_Scale2x(pixel *screen, pixel *mdScreen, int pitch, int x, int y, int offset)
{
	int i, j;
	pixel B, D, E, F, H;
	pixel E0, E1, E2, E3;
	
	// A B C
	// D E F
	// G H I
	
	int ScrWdth, ScrAdd;
	int SrcOffs, DstOffs;
	
	ScrAdd = offset >> 1;
	ScrWdth = x + ScrAdd;
	
	SrcOffs = 8;
	DstOffs = 0;
	
	for (i = 0; i < y; i++)
	{
		E = mdScreen[SrcOffs - 1];
		F = mdScreen[SrcOffs];
		
		DstOffs = i * (pitch / sizeof(pixel)) * 2;
		
		for (j = 0; j < x; j++)
		{
			D = E; E = F;
			B = mdScreen[SrcOffs - ScrWdth];
			H = mdScreen[SrcOffs + ScrWdth];
			F = mdScreen[++SrcOffs];
			E0 = D == B && B != F && D != H ? D : E;
			E1 = B == F && B != D && F != H ? F : E;
			E2 = D == H && D != B && H != F ? D : E;
			E3 = H == F && D != H && B != F ? F : E;
			
			// Copy the new pixels.
			screen[DstOffs + 0] = E0;
			screen[DstOffs + 1] = E1;
			screen[DstOffs + (pitch / sizeof(pixel)) + 0] = E2;
			screen[DstOffs + (pitch / sizeof(pixel)) + 1] = E3;
			
			// Next group of pixels.
			DstOffs += 2;
		}
		
		SrcOffs += ScrAdd;
	}
}

void Blit_Scale2x_16(unsigned char *screen, int pitch, int x, int y, int offset)
{
	Blit_Scale2x((unsigned short*)screen, MD_Screen, pitch, x, y, offset);
}

void Blit_Scale2x_32(unsigned char *screen, int pitch, int x, int y, int offset)
{
	Blit_Scale2x((unsigned int*)screen, MD_Screen32, pitch, x, y, offset);
}
