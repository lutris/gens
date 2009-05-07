/***************************************************************************
 * MDP: hq2x renderer. (RGB lookup tables)                                 *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2009 by David Korth                                  *
 * hq2x Copyright (c) 2003 by Maxim Stepin                                 *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU Lesser General Public License as published   *
 * by the Free Software Foundation; either version 2.1 of the License, or  *
 * (at your option) any later version.                                     *
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

#include "mdp_render_hq2x_RGB.h"
#include "mdp/mdp_stdint.h"

// C includes.
#include <stdlib.h>


/**
 * mdp_render_hq2x_build_RGB16to32(): Build a 16-bit RGB to 32-bit RGB table.
 * @return RGB16to32.
 */
uint32_t* MDP_FNCALL mdp_render_hq2x_build_RGB16to32(void)
{
	uint32_t *RGB16to32 = (uint32_t*)(malloc(65536 * sizeof(uint32_t)));
	
	// Initialize the 16-bit to 32-bit conversion table.
	int i;
	for (i = 0; i < 65536; i++)
		RGB16to32[i] = ((i & 0xF800) << 8) + ((i & 0x07E0) << 5) + ((i & 0x001F) << 3);
	
	// Return the pointer.
	return RGB16to32;
}


/**
 * mdp_render_hq2x_build_RGB16toYUV(): Build a 16-bit RGB to YUV table.
 * @return RGB16toYUV.
 */
uint32_t* MDP_FNCALL mdp_render_hq2x_build_RGB16toYUV(void)
{
	uint32_t *RGB16toYUV = (uint32_t*)(malloc(65536 * sizeof(uint32_t)));
	
	// Initialize the 16-bit RGB to YUV conversion table.
	int i, j, k, r, g, b, Y, u, v;
	
	for (i = 0; i < 32; i++)
	{
		for (j = 0; j < 64; j++)
		{
			for (k = 0; k < 32; k++)
			{
				r = i << 3;
				g = j << 2;
				b = k << 3;
				Y = (r + g + b) >> 2;
				u = 128 + ((r - b) >> 2);
				v = 128 + ((-r + 2*g -b) >> 3);
				RGB16toYUV[(i << 11) + (j << 5) + k] = (Y << 16) + (u << 8) + v;
			}
		}
	}
	
	// Return the pointer.
	return RGB16toYUV;
}
