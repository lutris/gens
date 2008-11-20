/***************************************************************************
 * Gens: [MDP] 2x renderer.                                                *
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

#include "mdp_render_2x.hpp"
#include <string.h>
#include <stdint.h>

// TODO: Add a plugin-specific version of the CPU Flags file.
#include "gens_core/misc/cpuflags.h"

// x86 asm versions
#ifdef GENS_X86_ASM
#include "mdp_render_2x_x86.h"
#endif /* GENS_X86_ASM */


#ifndef GENS_X86_ASM
/**
 * mdp_render_2x_cpp_int: Blits the image to the screen, 1x size, no filtering.
 * @param destScreen Pointer to the destination screen buffer.
 * @param mdScreen Pointer to the MD screen buffer.
 * @param width Width of the image.
 * @param height Height of the image.
 * @param pitch Number of bytes per line.
 * @param offset ???
 */
template<typename pixel>
static inline void mdp_render_2x_cpp_int(pixel *destScreen, pixel *mdScreen,
					 int width, int height,
					 int pitch, int offset)
{
	// Pitch difference.
	pitch /= sizeof(pixel);
	int nextLine = pitch + (pitch - (width * 2));
	
	pixel *line1 = destScreen;
	for (unsigned short i = 0; i < height; i++)
	{
		//dstOffs = i * (pitch / sizeof(pixel)) * 2;
		for (unsigned short j = 0; j < width; j++)
		{
			*line1++ = *mdScreen;
			*line1++ = *mdScreen++;
		}
		
		// Next line.
		mdScreen += offset;
		line1 += nextLine;
	}
	
	// Copy lines
	line1 = destScreen;
	pixel *line2 = destScreen + pitch;
	for (unsigned short i = 0; i < height; i++)
	{
		memcpy(line2, line1, pitch * sizeof(pixel));
		line1 += (pitch * 2);
		line2 += (pitch * 2);
	}
}
#endif /* GENS_X86_ASM */


void mdp_render_2x_cpp(MDP_Render_Info_t *renderInfo)
{
	if (!renderInfo)
		return;
	
	if (renderInfo->bpp == 15 || renderInfo->bpp == 16)
	{
#ifdef GENS_X86_ASM
		if (renderInfo->cpuFlags & CPUFLAG_MMX)
		{
			mdp_render_2x_16_x86_mmx(
				    (uint16_t*)renderInfo->destScreen,
				    (uint16_t*)renderInfo->mdScreen,
				    renderInfo->width, renderInfo->height,
				    renderInfo->pitch, renderInfo->offset);
		}
		else
		{
			mdp_render_2x_16_x86(
				    (uint16_t*)renderInfo->destScreen,
				    (uint16_t*)renderInfo->mdScreen,
				    renderInfo->width, renderInfo->height,
				    renderInfo->pitch, renderInfo->offset);
		}
#else /* !GENS_X86_ASM */
		mdp_render_2x_cpp_int(
			    (uint16_t*)renderInfo->destScreen,
			    (uint16_t*)renderInfo->mdScreen,
			    renderInfo->width, renderInfo->height,
			    renderInfo->pitch, renderInfo->offset);
#endif /* GENS_X86_ASM */
	}
	else
	{
#ifdef GENS_X86_ASM
		if (renderInfo->cpuFlags & CPUFLAG_MMX)
		{
			mdp_render_2x_32_x86_mmx(
				    (uint32_t*)renderInfo->destScreen,
				    (uint32_t*)renderInfo->mdScreen,
				    renderInfo->width, renderInfo->height,
				    renderInfo->pitch, renderInfo->offset);
		}
		else
		{
			mdp_render_2x_32_x86(
				    (uint32_t*)renderInfo->destScreen,
				    (uint32_t*)renderInfo->mdScreen,
				    renderInfo->width, renderInfo->height,
				    renderInfo->pitch, renderInfo->offset);
		}
#else /* !GENS_X86_ASM */
		mdp_render_2x_cpp_int(
			    (uint32_t*)renderInfo->destScreen,
			    (uint32_t*)renderInfo->mdScreen,
			    renderInfo->width, renderInfo->height,
			    renderInfo->pitch, renderInfo->offset);
#endif /* GENS_X86_ASM */
	}
}
