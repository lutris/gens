/***************************************************************************
 * MDP: Interpolated renderer.                                             *
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

#include "mdp_render_interpolated.hpp"
#include "mdp_render_interpolated_plugin.h"

// MDP includes.
#include "mdp/mdp_stdint.h"
#include "mdp/mdp_cpuflags.h"
#include "mdp/mdp_error.h"

// Mask constants.
#define MASK_DIV2_15		((uint16_t)(0x3DEF))
#define MASK_DIV2_16		((uint16_t)(0x7BEF))
#define MASK_DIV2_32		((uint32_t)(0x7F7F7F7F))

// MDP Host Services.
static const mdp_host_t *mdp_render_interpolated_host_srv = NULL;


/**
 * mdp_render_interpolated_init(): Initialize the Interpolated rendering plugin.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_render_interpolated_init(const mdp_host_t *host_srv)
{
	if (!host_srv)
		return -MDP_ERR_INVALID_PARAMETERS;
	
	// Save the MDP Host Services pointer.
	mdp_render_interpolated_host_srv = host_srv;
	
	// Register the renderer.
	return mdp_render_interpolated_host_srv->renderer_register(&mdp, &mdp_render);
}


/**
 * mdp_render_interpolated_end(): Shut down the Interpolated rendering plugin.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_render_interpolated_end(void)
{
	if (!mdp_render_interpolated_host_srv)
		return MDP_ERR_OK;
	
	// Unregister the renderer.
	mdp_render_interpolated_host_srv->renderer_unregister(&mdp, &mdp_render);
	
	// Plugin is shut down.
	return MDP_ERR_OK;
}


/**
 * T_mdp_render_interpolated_cpp: Blits the image to the screen, 2x size, interpolation.
 * @param destScreen Pointer to the destination screen buffer.
 * @param mdScreen Pointer to the MD screen buffer.
 * @param destPitch Pitch of destScreen.
 * @param srcPitch Pitch of mdScreen.
 * @param width Width of the image.
 * @param height Height of the image.
 * @param mask Mask for the interpolation data.
 */
template<typename pixel>
static inline void T_mdp_render_interpolated_cpp(pixel *destScreen, pixel *mdScreen,
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
			pixel D = *(SrcLine + srcPitch);
			pixel DR = *(SrcLine + srcPitch + 1);
			
			*DstLine1++ = C;
			*DstLine1++ = ((C >> 1) & mask) + ((R >> 1) & mask);
			*DstLine2++ = ((C >> 1) & mask) + ((D >> 1) & mask);
			*DstLine2++ = (((((C >> 1) & mask) + ((R >> 1) & mask)) >> 1) & mask) +
				      (((((D >> 1) & mask) + ((DR >> 1) & mask)) >> 1) & mask);
			
			SrcLine++;
		}
	}
}


int MDP_FNCALL mdp_render_interpolated_cpp(const mdp_render_info_t *render_info)
{
	if (!render_info)
		return -MDP_ERR_RENDER_INVALID_RENDERINFO;;
	
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
			T_mdp_render_interpolated_cpp(
				    (uint16_t*)render_info->destScreen,
				    (uint16_t*)render_info->mdScreen,
				    render_info->destPitch, render_info->srcPitch,
				    render_info->width, render_info->height,
				    (MDP_RENDER_VMODE_GET_SRC(render_info->vmodeFlags) == MDP_RENDER_VMODE_RGB_565 ? MASK_DIV2_16 : MASK_DIV2_15));
			break;
		
		case MDP_RENDER_VMODE_RGB_888:
			T_mdp_render_interpolated_cpp(
				    (uint32_t*)render_info->destScreen,
				    (uint32_t*)render_info->mdScreen,
				    render_info->destPitch, render_info->srcPitch,
				    render_info->width, render_info->height,
				    MASK_DIV2_32);
			break;
		
		default:
			// Unsupported video mode.
			return -MDP_ERR_RENDER_UNSUPPORTED_VMODE;
	}
	
	return MDP_ERR_OK;
}
