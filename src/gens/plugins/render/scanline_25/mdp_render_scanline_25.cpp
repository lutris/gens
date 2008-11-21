/***************************************************************************
 * Gens: [MDP] 25% Scanline renderer.                                      *
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

#include "mdp_render_scanline_25.hpp"
#include <string.h>
#include <stdint.h>

// TODO: Add a plugin-specific version of the CPU Flags file.
#include "gens_core/misc/cpuflags.h"

// x86 asm versions
#undef GENS_X86_ASM
#ifdef GENS_X86_ASM
#include "mdp_render_scanline_25_x86.h"
#endif /* GENS_X86_ASM */

// Mask constants
#define MASK_DIV2_15 ((uint16_t)(0x3DEF))
#define MASK_DIV2_16 ((uint16_t)(0x7BEF))
#define MASK_DIV2_15_ASM ((uint32_t)(0x3DEF3DEF))
#define MASK_DIV2_16_ASM ((uint32_t)(0x7BEF7BEF))
#define MASK_DIV2_32 ((uint32_t)(0x007F7F7F))

#define MASK_DIV4_15 ((uint16_t)(0x1CE7))
#define MASK_DIV4_16 ((uint16_t)(0x39E7))
#define MASK_DIV4_15_ASM ((uint32_t)(0x1CE71CE7))
#define MASK_DIV4_16_ASM ((uint32_t)(0x39E739E7))
#define MASK_DIV4_32 ((uint32_t)(0x003F3F3f))


#ifndef GENS_X86_ASM
/**
 * T_mdp_render_scanline_25_cpp: Blits the image to the screen, 2x size, 25% scanlines.
 * @param screen Pointer to the screen buffer.
 * @param mdScreen Pointer to the MD screen buffer.
 * @param pitch Number of bytes per line.
 * @param x X coordinate for the image.
 * @param y Y coordinate for the image.
 * @param offset ???
 * @param mask Mask for the scanline data.
 */
template<typename pixel>
static inline void T_mdp_render_scanline_25_cpp(pixel *destScreen, pixel *mdScreen,
						int width, int height,
						int pitch, int offset,
						pixel mask2, pixel mask4)
{
	// Pitch difference.
	pitch /= sizeof(pixel);
	int nextLine = pitch + (pitch - (width * 2));
	
	pixel *line1 = destScreen;
	pixel *line2 = destScreen + pitch;
	for (unsigned short i = 0; i < height; i++)
	{
		for (unsigned short j = 0; j < width; j++)
		{
			pixel tmp = *mdScreen++;
			
			// First line.
			*line1++ = tmp;
			*line1++ = tmp;
			
			// Second line. (25% Scanline effect)
			tmp = ((tmp >> 1) & mask2) + ((tmp >> 2) & mask4);
			*line2++ = tmp;
			*line2++ = tmp;
		}
		
		// Next line.
		mdScreen += offset;
		line1 += nextLine;
		line2 += nextLine;
	}
}
#endif /* GENS_X86_ASM */


/**
 * 
 * @param renderInfo 
 */
void mdp_render_scanline_25_cpp(MDP_Render_Info_t *renderInfo)
{
	if (!renderInfo)
		return;
	
	if (renderInfo->bpp == 16 || renderInfo->bpp == 15)
	{
#ifdef GENS_X86_ASM
		if (renderInfo->cpuFlags & CPUFLAG_MMX)
		{
			mdp_render_scanline_25_16_x86_mmx(
				    (uint16_t*)renderInfo->destScreen,
				    (uint16_t*)renderInfo->mdScreen,
				    renderInfo->width, renderInfo->height,
				    renderInfo->pitch, renderInfo->offset,
				    (renderInfo->bpp == 15));
		}
		else
		{
			mdp_render_scanline_25_16_x86(
				    (uint16_t*)renderInfo->destScreen,
				    (uint16_t*)renderInfo->mdScreen,
				    renderInfo->width, renderInfo->height,
				    renderInfo->pitch, renderInfo->offset,
				    (renderInfo->bpp == 16 ? MASK_DIV2_16_ASM : MASK_DIV2_15_ASM),
				    (renderInfo->bpp == 16 ? MASK_DIV4_16_ASM : MASK_DIV4_15_ASM));
		}
#else /* !GENS_X86_ASM */
		T_mdp_render_scanline_25_cpp(
			    (uint16_t*)renderInfo->destScreen,
			    (uint16_t*)renderInfo->mdScreen,
			    renderInfo->width, renderInfo->height,
			    renderInfo->pitch, renderInfo->offset,
			    (renderInfo->bpp == 16 ? MASK_DIV2_16 : MASK_DIV2_15),
			    (renderInfo->bpp == 16 ? MASK_DIV4_16 : MASK_DIV4_15));
#endif /* GENS_X86_ASM */
	}
	else
	{
#ifdef GENS_X86_ASM
		if (renderInfo->cpuFlags & CPUFLAG_MMX)
		{
			mdp_render_scanline_25_32_x86_mmx(
				    (uint32_t*)renderInfo->destScreen,
				    (uint32_t*)renderInfo->mdScreen,
				    renderInfo->width, renderInfo->height,
				    renderInfo->pitch, renderInfo->offset);
		}
		else
		{
			mdp_render_scanline_25_32_x86(
				    (uint32_t*)renderInfo->destScreen,
				    (uint32_t*)renderInfo->mdScreen,
				    renderInfo->width, renderInfo->height,
				    renderInfo->pitch, renderInfo->offset);
		}
#else /* !GENS_X86_ASM */
		T_mdp_render_scanline_25_cpp(
			    (uint32_t*)renderInfo->destScreen,
			    (uint32_t*)renderInfo->mdScreen,
			    renderInfo->width, renderInfo->height,
			    renderInfo->pitch, renderInfo->offset,
			    MASK_DIV2_32, MASK_DIV4_32);
#endif /* GENS_X86_ASM */
	}
}
