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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// CPU flags
#include "plugins/mdp_cpuflags.h"

// x86 asm versions
#ifdef GENS_X86_ASM
#include "mdp_render_hq2x_x86.h"
#endif /* GENS_X86_ASM */

// hq2x lookup tables
int *mdp_render_hq2x_LUT16to32 = NULL;
int *mdp_render_hq2x_RGBtoYUV = NULL;

// TODO: Proper 15-bit color support.
// It "works' right now in 15-bit, but the output is different than with 16-bit.
// My first attempt at 15-bit support resulted in massive failure. :(


/**
 * mdp_render_hq2x_end(): Shut down the hq2x plugin.
 */
void mdp_render_hq2x_end(void)
{
	// Free all lookup tables.
	if (mdp_render_hq2x_LUT16to32)
	{
		free(mdp_render_hq2x_LUT16to32);
		mdp_render_hq2x_LUT16to32 = NULL;
	}
	
	if (mdp_render_hq2x_RGBtoYUV)
	{
		free(mdp_render_hq2x_RGBtoYUV);
		mdp_render_hq2x_RGBtoYUV = NULL;
	}
}


/**
 * mdp_render_hq2x_InitLUT16to32(): Initialize LUT16to32.
 */
static void mdp_render_hq2x_InitLUT16to32(void)
{
	// Allocate the memory for the lookup table.
	mdp_render_hq2x_LUT16to32 = malloc(65536 * sizeof(int));
	
	// Initialize the 16-bit to 32-bit conversion table.
	for (int i = 0; i < 65536; i++)
		mdp_render_hq2x_LUT16to32[i] = ((i & 0xF800) << 8) + ((i & 0x07E0) << 5) + ((i & 0x001F) << 3);
}


/**
 * mdp_render_hq2x_InitRGBtoYUV(): Initialize RGBtoYUV.
 */
static void mdp_render_hq2x_InitRGBtoYUV(void)
{
	// Allocate the memory for the lookup table.
	mdp_render_hq2x_RGBtoYUV = malloc(65536 * sizeof(int));
	
	// Initialize the RGB to YUV conversion table.
	int r, g, b, Y, u, v;
	
	for (int i = 0; i < 32; i++)
	{
		for (int j = 0; j < 64; j++)
		{
			for (int k = 0; k < 32; k++)
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


void mdp_render_hq2x_cpp(MDP_Render_Info_t *renderInfo)
{
	if (!renderInfo)
		return;
	
	// Make sure the lookup tables are initialized.
	if (!mdp_render_hq2x_LUT16to32)
		mdp_render_hq2x_InitLUT16to32();
	if (!mdp_render_hq2x_RGBtoYUV)
		mdp_render_hq2x_InitRGBtoYUV();
	
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
	else
	{
#ifdef GENS_X86_ASM
		if (renderInfo->cpuFlags & MDP_CPUFLAG_MMX)
		{
			mdp_render_hq2x_32_x86_mmx(
				    (uint32_t*)renderInfo->destScreen,
				    (uint16_t*)renderInfo->mdScreen,
				    renderInfo->width, renderInfo->height,
				    renderInfo->pitch, renderInfo->offset);
		}
		/*
		else
		{
			mdp_render_hq2x_32_x86(
				    (uint32_t*)renderInfo->destScreen,
				    (uint32_t*)renderInfo->mdScreen,
				    renderInfo->width, renderInfo->height,
				    renderInfo->pitch, renderInfo->offset);
		}
		*/
#else /* !GENS_X86_ASM */
		T_mdp_render_hq2x_cpp(
			    (uint32_t*)renderInfo->destScreen,
			    (uint32_t*)renderInfo->mdScreen,
			    renderInfo->width, renderInfo->height,
			    renderInfo->pitch, renderInfo->offset);
#endif /* GENS_X86_ASM */
	}
}
