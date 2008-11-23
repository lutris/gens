/***************************************************************************
 * Gens: [MDP] hq2x renderer.                                              *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "mdp_render_hq2x.h"
#include "mdp_render_hq2x_lut.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// CPU flags
#include "plugins/mdp_cpuflags.h"

// x86 asm versions
#ifdef GENS_X86_ASM
#include "mdp_render_hq2x_x86.h"
#endif /* GENS_X86_ASM */

// TODO: Proper 15-bit color support.
// It "works' right now in 15-bit, but the output is different than with 16-bit.
// My first attempt at 15-bit support resulted in massive failure. :(


/**
 * mdp_render_hq2x_end(): Initialize the hq2x plugin.
 */
void mdp_render_hq2x_init(void)
{
	// Increment the lookup table reference counter.
	mdp_render_hq2x_refcount++;
}


/**
 * mdp_render_hq2x_end(): Shut down the hq2x plugin.
 */
void mdp_render_hq2x_end(void)
{
	// Decrement the lookup table reference counter.
	mdp_render_hq2x_refcount--;
	
	if (mdp_render_hq2x_refcount == 0)
	{
		// This is the last plugin to use the hq2x lookup tables.
		// Deallocate them.
		
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
#else /* !GENS_X86_ASM */
		T_mdp_render_hq2x_cpp(
			    (uint32_t*)renderInfo->destScreen,
			    (uint32_t*)renderInfo->mdScreen,
			    renderInfo->width, renderInfo->height,
			    renderInfo->pitch, renderInfo->offset);
#endif /* GENS_X86_ASM */
	}
}
