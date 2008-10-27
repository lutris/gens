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

#include "plugins/plugin.h"
#include <string.h>
#include <stdint.h>


/**
 * Blit1x: Blits the image to the screen, 1x size, no filtering.
 * @param screen Pointer to the screen buffer.
 * @param mdScreen Pointer to the MD screen buffer.
 * @param pitch Number of bytes per line.
 * @param x X coordinate for the image.
 * @param y Y coordinate for the image.
 * @param offset ???
 */
template<typename pixel>
static inline void Blit1x_int(pixel *screen, pixel *mdScreen, int pitch, int x, int y, int offset)
{
	int i;
	
	// Adjust for the 8px border on the MD Screen.
	mdScreen += 8;
	
	for (i = 0; i < y; i++)
	{
		memcpy(screen, mdScreen, x * sizeof(pixel));
		
		// Next line.
		// TODO: Make this a constant somewhere.
		mdScreen += 336;
		screen += (pitch / sizeof(pixel));
	}
}


void Blit1x(MDP_Render_Info_t *renderInfo)
{
	if (!renderInfo)
		return;
	
	if (renderInfo->bpp == 15 || renderInfo->bpp == 16)
	{
		Blit1x_int((uint16_t*)renderInfo->screen,
			   renderInfo->screen16, renderInfo->pitch,
			   renderInfo->x, renderInfo->y, renderInfo->offset);
	}
	else
	{
		Blit1x_int((uint32_t*)renderInfo->screen,
			    renderInfo->screen32, renderInfo->pitch,
			    renderInfo->x, renderInfo->y, renderInfo->offset);
	}
}
