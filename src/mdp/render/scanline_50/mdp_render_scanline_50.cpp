/***************************************************************************
 * MDP: 50% Scanline renderer.                                             *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2009 by David Korth                                  *
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

#include "mdp_render_scanline_50.hpp"
#include "mdp_render_scanline_50_plugin.h"

// MDP includes.
#include "mdp/mdp_stdint.h"
#include "mdp/mdp_cpuflags.h"
#include "mdp/mdp_error.h"

// x86 asm versions.
#ifdef GENS_X86_ASM
#include "mdp_render_scanline_50_x86.h"
#endif /* GENS_X86_ASM */

// Mask constants.
#define MASK_DIV2_15		((uint16_t)(0x3DEF))
#define MASK_DIV2_16		((uint16_t)(0x7BEF))
#define MASK_DIV2_15_ASM	((uint32_t)(0x3DEF3DEF))
#define MASK_DIV2_16_ASM	((uint32_t)(0x7BEF7BEF))
#define MASK_DIV2_32		((uint32_t)(0x7F7F7F7F))

// MDP Host Services.
static const mdp_host_t *mdp_render_scanline_50_host_srv = NULL;


/**
 * mdp_render_scanline_50_init(): Initialize the 50% Scanline rendering plugin.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_render_scanline_50_init(const mdp_host_t *host_srv)
{
	if (!host_srv)
		return -MDP_ERR_INVALID_PARAMETERS;
	
	// Save the MDP Host Services pointer.
	mdp_render_scanline_50_host_srv = host_srv;
	
	// Register the renderer.
	return mdp_render_scanline_50_host_srv->renderer_register(&mdp, &mdp_render);
}


/**
 * mdp_render_scanline_50_end(): Shut down the Scanline rendering plugin.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_render_scanline_50_end(void)
{
	if (!mdp_render_scanline_50_host_srv)
		return MDP_ERR_OK;
	
	// Unregister the renderer.
	mdp_render_scanline_50_host_srv->renderer_unregister(&mdp, &mdp_render);
	
	// Plugin is shut down.
	return MDP_ERR_OK;
}


#ifndef GENS_X86_ASM
/**
 * T_mdp_render_scanline_50_cpp: Blits the image to the screen, 2x size, 50% scanlines.
 * @param screen Pointer to the screen buffer.
 * @param mdScreen Pointer to the MD screen buffer.
 * @param destPitch Pitch of destScreen.
 * @param srcPitch Pitch of mdScreen.
 * @param width Width of the image.
 * @param height Height of the image.
 * @param mask Mask for the scanline data.
 */
template<typename pixel>
static inline void T_mdp_render_scanline_50_cpp(pixel *destScreen, pixel *mdScreen,
					     int destPitch, int srcPitch,
					     int width, int height, pixel mask)
{
	// Pitch difference.
	destPitch /= sizeof(pixel);
	int nextLine = destPitch + (destPitch - (width * 2));
	
	srcPitch /= sizeof(pixel);
	
	pixel *line1 = destScreen;
	pixel *line2 = destScreen + destPitch;
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			pixel tmp = *mdScreen++;
			
			// First line.
			*line1++ = tmp;
			*line1++ = tmp;
			
			// Second line. (50% Scanline effect)
			tmp = (tmp >> 1) & mask;
			*line2++ = tmp;
			*line2++ = tmp;
		}
		
		// Next line.
		mdScreen += (srcPitch - width);
		line1 += nextLine;
		line2 += nextLine;
	}
}
#endif /* GENS_X86_ASM */


int MDP_FNCALL mdp_render_scanline_50_cpp(const mdp_render_info_t *render_info)
{
	if (!render_info)
		return -MDP_ERR_RENDER_INVALID_RENDERINFO;
	
	if (MDP_RENDER_VMODE_GET_SRC(render_info->vmodeFlags) !=
	    MDP_RENDER_VMODE_GET_DST(render_info->vmodeFlags))
	{
		// Renderer only supports identical src/dst modes.
		return -MDP_ERR_RENDER_UNSUPPORTED_VMODE;
	}
	
	switch (MDP_RENDER_VMODE_GET_SRC(render_info->vmodeFlags))
	{
		case MDP_RENDER_VMODE_RGB_555:
		case MDP_RENDER_VMODE_RGB_565:
		{
			const int mode565 = ((MDP_RENDER_VMODE_GET_SRC(render_info->vmodeFlags) == MDP_RENDER_VMODE_RGB_565) ? 1 : 0);
			
#ifdef GENS_X86_ASM
			if (render_info->cpuFlags & MDP_CPUFLAG_X86_MMX)
			{
				mdp_render_scanline_50_16_x86_mmx(
					    (uint16_t*)render_info->destScreen,
					    (uint16_t*)render_info->mdScreen,
					    render_info->destPitch, render_info->srcPitch,
					    render_info->width, render_info->height,
					    mode565);
			}
			else
			{
				mdp_render_scanline_50_16_x86(
					    (uint16_t*)render_info->destScreen,
					    (uint16_t*)render_info->mdScreen,
					    render_info->destPitch, render_info->srcPitch,
					    render_info->width, render_info->height,
					    (mode565 ? MASK_DIV2_16_ASM : MASK_DIV2_15_ASM));
			}
#else /* !GENS_X86_ASM */
			T_mdp_render_scanline_50_cpp(
				    (uint16_t*)render_info->destScreen,
				    (uint16_t*)render_info->mdScreen,
				    render_info->destPitch, render_info->srcPitch,
				    render_info->width, render_info->height,
				    (mode565 ? MASK_DIV2_16 : MASK_DIV2_15));
#endif /* GENS_X86_ASM */
			break;
		}
		case MDP_RENDER_VMODE_RGB_888:
#ifdef GENS_X86_ASM
			if (render_info->cpuFlags & MDP_CPUFLAG_X86_MMX)
			{
				mdp_render_scanline_50_32_x86_mmx(
					    (uint32_t*)render_info->destScreen,
					    (uint32_t*)render_info->mdScreen,
					    render_info->destPitch, render_info->srcPitch,
					    render_info->width, render_info->height);
			}
			else
			{
				mdp_render_scanline_50_32_x86(
					    (uint32_t*)render_info->destScreen,
					    (uint32_t*)render_info->mdScreen,
					    render_info->destPitch, render_info->srcPitch,
					    render_info->width, render_info->height);
			}
#else /* !GENS_X86_ASM */
			T_mdp_render_scanline_50_cpp(
				    (uint32_t*)render_info->destScreen,
				    (uint32_t*)render_info->mdScreen,
				    render_info->destPitch, render_info->srcPitch,
				    render_info->width, render_info->height,
				    MASK_DIV2_32);
#endif /* GENS_X86_ASM */
			break;
		
		default:
			// Unsupported video mode.
			return -MDP_ERR_RENDER_UNSUPPORTED_VMODE;
	}
	
	return MDP_ERR_OK;
}
