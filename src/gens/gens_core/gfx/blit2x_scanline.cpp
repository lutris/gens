/***************************************************************************
 * Gens: 2x scanline renderer.                                             *
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
#include "gens_core/vdp/vdp_rend.h"
#include "blit.h"

/**
 * Blit2x_Scanline: Blits the image to the screen, 2x size, scanlines.
 * @param screen Pointer to the screen buffer.
 * @param mdScreen Pointer to the MD screen buffer.
 * @param pitch Number of bytes per line.
 * @param x X coordinate for the image.
 * @param y Y coordinate for the image.
 * @param offset ???
 */
template<typename pixel>
static inline void Blit2x_Scanline(pixel *screen, pixel *mdScreen, int pitch, unsigned short x, unsigned short y, int offset)
{
	// Adjust for the 8px border on the MD Screen.
	mdScreen += 8;
	
	// Pitch difference.
	pitch /= sizeof(pixel);
	int nextLine = pitch + (pitch - (x * 2));
	
	offset >>= 1;
	
	pixel *line = screen;
	for (unsigned short i = 0; i < y; i++)
	{
		for (unsigned short j = 0; j < x; j++)
		{
			*line++ = *mdScreen;
			*line++ = *mdScreen++;
		}
		
		// Next line.
		mdScreen += offset;
		line += nextLine;
	}
	
	// Zero the extra lines.
	line = screen + pitch;
	for (unsigned short i = 0; i < y; i++)
	{
		memset(line, 0x00, pitch * sizeof(pixel));
		line += (pitch * 2);
	}
}

#ifndef GENS_X86_ASM
void Blit2x_Scanline_16(unsigned char *screen, int pitch, int x, int y, int offset)
{
	Blit2x_Scanline((unsigned short*)screen, MD_Screen, pitch, x, y, offset);
}
#endif

void Blit2x_Scanline_32(unsigned char *screen, int pitch, int x, int y, int offset)
{
	Blit2x_Scanline((unsigned int*)screen, MD_Screen32, pitch, x, y, offset);
}
