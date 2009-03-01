/***************************************************************************
 * Gens: [MDP] Interpolated Scanline renderer.                             *
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

#include <stdint.h>
#include <string.h>

#include "mdp_render_interpolated_scanline.hpp"
#include "mdp_render_interpolated_scanline_plugin.h"

// MDP includes.
#include "mdp/mdp_cpuflags.h"
#include "mdp/mdp_error.h"

// x86 asm versions.
#ifdef GENS_X86_ASM
#include "mdp_render_interpolated_scanline_x86.h"
#endif /* GENS_X86_ASM */

// Mask constants.
#define MASK_DIV2_15 		((uint16_t)(0x3DEF))
#define MASK_DIV2_16		((uint16_t)(0x7BEF))
#define MASK_DIV2_15_ASM	((uint32_t)(0x3DEF3DEF))
#define MASK_DIV2_16_ASM	((uint32_t)(0x7BEF7BEF))
#define MASK_DIV2_32		((uint32_t)(0x7F7F7F7F))

// MDP Host Services.
static mdp_host_t *mdp_render_interpolated_scanline_host_srv = NULL;


/**
 * mdp_render_interpolated_scanline_init(): Initialize the Interpolated Scanline rendering plugin.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_render_interpolated_scanline_init(mdp_host_t *host_srv)
{
	// Save the MDP Host Services pointer.
	mdp_render_interpolated_scanline_host_srv = host_srv;
	
	// Register the renderer.
	mdp_render_interpolated_scanline_host_srv->renderer_register(&mdp, &mdp_render);
	
	// Initialized.
	return MDP_ERR_OK;
}


/**
 * mdp_render_interpolated_scanline_end(): Shut down the Interpolated Scanline rendering plugin.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_render_interpolated_scanline_end(void)
{
	if (!mdp_render_interpolated_scanline_host_srv)
		return MDP_ERR_OK;
	
	// Unregister the renderer.
	mdp_render_interpolated_scanline_host_srv->renderer_unregister(&mdp, &mdp_render);
	
	// Plugin is shut down.
	return MDP_ERR_OK;
}


//#ifndef GENS_X86_ASM
/**
 * T_mdp_render_interpolated_scanline_cpp: Blits the image to the screen, 2x size, interpolation with scanlines.
 * @param destScreen Pointer to the destination screen buffer.
 * @param mdScreen Pointer to the MD screen buffer.
 * @param destPitch Pitch of destScreen.
 * @param srcPitch Pitch of mdScreen.
 * @param width Width of the image.
 * @param height Height of the image.
 * @param mask Mask for the interpolation data.
 */
template<typename pixel>
static inline void T_mdp_render_interpolated_scanline_cpp(pixel *destScreen, pixel *mdScreen,
						 int destPitch, int srcPitch,
						 int width, int height, pixel mask)
{
	destPitch /= sizeof(pixel);
	srcPitch /= sizeof(pixel);
	
	for (int y = 0; y < height; y++)
	{
		pixel *SrcLine = &mdScreen[y * srcPitch];
		pixel *DstLine1 = &destScreen[(y * 2) * destPitch];
		pixel *DstLine2 = &destScreen[((y * 2) + 1) * destPitch];
		
		for (int x = 0; x < width; x++)
		{
			pixel C = *(SrcLine);
			pixel R = *(SrcLine + 1);
			
			*DstLine1++ = C;
			*DstLine1++ = ((C >> 1) & mask) + ((R >> 1) & mask);
			*DstLine2++ = 0;
			*DstLine2++ = 0;
			
			SrcLine++;
		}
	}
}
//#endif /* GENS_X86_ASM */


int MDP_FNCALL mdp_render_interpolated_scanline_cpp(mdp_render_info_t *render_info)
{
	if (!render_info)
		return -MDP_ERR_RENDER_INVALID_RENDERINFO;
	
	if (render_info->bpp == 16 || render_info->bpp == 15)
	{
#ifdef GENS_X86_ASM
		if (render_info->cpuFlags & MDP_CPUFLAG_MMX)
		{
			mdp_render_interpolated_scanline_16_x86_mmx(
				    (uint16_t*)render_info->destScreen,
				    (uint16_t*)render_info->mdScreen,
				    render_info->destPitch, render_info->srcPitch,
				    render_info->width, render_info->height,
				    (render_info->bpp == 15));
		}
		else
		{
			mdp_render_interpolated_scanline_16_x86(
				    (uint16_t*)render_info->destScreen,
				    (uint16_t*)render_info->mdScreen,
				    render_info->destPitch, render_info->srcPitch,
				    render_info->width, render_info->height,
				    (render_info->bpp == 15 ? MASK_DIV2_15_ASM : MASK_DIV2_16_ASM));
		}
#else /* !GENS_X86_ASM */
		T_mdp_render_interpolated_scanline_cpp(
			    (uint16_t*)render_info->destScreen,
			    (uint16_t*)render_info->mdScreen,
				    render_info->destPitch, render_info->srcPitch,
				    render_info->width, render_info->height,
			    (render_info->bpp == 15 ? MASK_DIV2_15 : MASK_DIV2_16));
#endif /* GENS_X86_ASM */
	}
	else
	{
#if 0
#ifdef GENS_X86_ASM
		if (render_info->cpuFlags & CPUFLAG_MMX)
		{
			mdp_render_interpolated_scanline_32_x86_mmx(
				    (uint32_t*)render_info->destScreen,
				    (uint32_t*)render_info->mdScreen,
				    render_info->destPitch, render_info->srcPitch,
				    render_info->width, render_info->height);
		}
		else
		{
			mdp_render_interpolated_scanline_32_x86(
				    (uint32_t*)render_info->destScreen,
				    (uint32_t*)render_info->mdScreen,
				    render_info->destPitch, render_info->srcPitch,
				    render_info->width, render_info->height);
		}
#else /* !GENS_X86_ASM */
#endif
#endif
		T_mdp_render_interpolated_scanline_cpp(
			    (uint32_t*)render_info->destScreen,
			    (uint32_t*)render_info->mdScreen,
			    render_info->destPitch, render_info->srcPitch,
			    render_info->width, render_info->height,
			    MASK_DIV2_32);
//#endif /* GENS_X86_ASM */
	}
	
	return MDP_ERR_OK;
}
