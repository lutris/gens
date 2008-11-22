/***************************************************************************
 * Gens: [MDP] hq2x renderer.                                              *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
 * Scale2x Copyright (c) 2001 by Andrea Mazzoleni                          *
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

#include "mdp_render_hq2x.h"
#include <string.h>
#include <stdint.h>

// CPU flags
#include "plugins/mdp_cpuflags.h"

// x86 asm versions
#ifdef GENS_X86_ASM
#include "mdp_render_hq2x_x86.h"
#endif /* GENS_X86_ASM */

// hq2x lookup tables
int LUT16to32[65536];
int RGBtoYUV[65536];


// TODO: Only initialize the LUTs when necessary.
// Right now, this function is the MDP_t init() function.
void mdp_render_hq2x_InitLUTs(void)
{
	int i, j, k, r, g, b, Y, u, v;
	
	// Initialize the 16-bit to 32-bit conversion table.
	// TODO: 15-bit color support.
	for (i = 0; i < 65536; i++)
		LUT16to32[i] = ((i & 0xF800) << 8) + ((i & 0x07E0) << 5) + ((i & 0x001F) << 3);
	
	// Initialize the RGB to YUV conversion table.
	// TODO: 15-bit color support.
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
				RGBtoYUV[(i << 11) + (j << 5) + k] = (Y << 16) + (u << 8) + v;
			}
		}
	}
}


void mdp_render_hq2x_cpp(MDP_Render_Info_t *renderInfo)
{
	if (!renderInfo)
		return;

	if (renderInfo->bpp == 15 || renderInfo->bpp == 16)
	{
#ifdef GENS_X86_ASM
		if (renderInfo->cpuFlags & MDP_CPUFLAG_MMX)
		{
			mdp_render_hq2x_16_x86_mmx(
				    (uint16_t*)renderInfo->destScreen,
				    (uint16_t*)renderInfo->mdScreen,
				    renderInfo->width, renderInfo->height,
				    renderInfo->pitch, renderInfo->offset);
		}
		/*
		else
		{
			mdp_render_hq2x_16_x86(
				    (uint16_t*)renderInfo->destScreen,
				    (uint16_t*)renderInfo->mdScreen,
				    renderInfo->width, renderInfo->height,
				    renderInfo->pitch, renderInfo->offset);
		}
		*/
#else /* !GENS_X86_ASM */
		T_mdp_render_hq2x_cpp(
			    (uint16_t*)renderInfo->destScreen,
			    (uint16_t*)renderInfo->mdScreen,
			    renderInfo->width, renderInfo->height,
			    renderInfo->pitch, renderInfo->offset);
#endif /* GENS_X86_ASM */
	}
}
