/***************************************************************************
 * Gens: [MDP] hq2x renderer. (16-bit RGB to YUV lookup table)             *
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

#include "mdp_render_hq2x_RGB16toYUV.h"

// C includes.
#include <stdlib.h>

// Used for mprotect().
// TODO: Does mprotect() work on Win32?
#include <sys/mman.h>


/**
 * mdp_render_hq2x_build_rgb16toyuv(): Build a 16-bit RGB to YUV table.
 * @return RGB16toYUV.
 */
const int* MDP_FNCALL mdp_render_hq2x_build_RGB16toYUV(void)
{
	int *RGB16toYUV = (int*)(malloc(65536 * sizeof(int)));
	
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
	
	// Mark the lookup table as read-only.
	mprotect(RGB16toYUV, (65536 * sizeof(int)), PROT_READ);
	
	// Return the pointer.
	return RGB16toYUV;
}
