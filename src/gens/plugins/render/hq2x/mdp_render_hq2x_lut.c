/***************************************************************************
 * Gens: [MDP] hq2x renderer. (Lookup Tables)                              *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
 * hq2x Copyright (c) 2003 by Maxim Stepin                                 *
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


#include <string.h>
#include <stdlib.h>


// hq2x lookup tables.
int *mdp_render_hq2x_LUT16to32 = NULL;
int *mdp_render_hq2x_RGBtoYUV = NULL;
int mdp_render_hq2x_refcount = 0;


/**
 * mdp_render_hq2x_InitLUT16to32(): Initialize LUT16to32.
 */
void mdp_render_hq2x_InitLUT16to32(void)
{
	// Allocate the memory for the lookup table.
	mdp_render_hq2x_LUT16to32 = malloc(65536 * sizeof(int));
	
	// Initialize the 16-bit to 32-bit conversion table.
	int i;
	for (i = 0; i < 65536; i++)
		mdp_render_hq2x_LUT16to32[i] = ((i & 0xF800) << 8) + ((i & 0x07E0) << 5) + ((i & 0x001F) << 3);
}


/**
 * mdp_render_hq2x_InitRGBtoYUV(): Initialize RGBtoYUV.
 */
void mdp_render_hq2x_InitRGBtoYUV(void)
{
	// Allocate the memory for the lookup table.
	mdp_render_hq2x_RGBtoYUV = malloc(65536 * sizeof(int));
	
	// Initialize the RGB to YUV conversion table.
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
				mdp_render_hq2x_RGBtoYUV[(i << 11) + (j << 5) + k] = (Y << 16) + (u << 8) + v;
			}
		}
	}
}
