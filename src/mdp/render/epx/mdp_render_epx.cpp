/***************************************************************************
 * Gens: [MDP] EPX renderer.                                               *
 *                                                                         *
 * Original code from Gens Rerecording.                                    *
 * MDP version Copyright (c) 2008 by David Korth                           *
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

#include "mdp_render_epx.hpp"
#include "mdp_render_epx_plugin.h"

// MDP includes.
#include "mdp/mdp_error.h"

// MDP Host Services.
static MDP_Host_t *mdp_render_epx_hostSrv = NULL;


/**
 * mdp_render_epx_init(): Initialize the EPX rendering plugin.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_render_epx_init(MDP_Host_t *hostSrv)
{
	// Save the MDP Host Services pointer.
	mdp_render_epx_hostSrv = hostSrv;
	
	// Register the renderer.
	mdp_render_epx_hostSrv->renderer_register(&mdp, &mdp_render_t);
	
	// Initialized.
	return MDP_ERR_OK;
}


/**
 * mdp_render_epx_end(): Shut down the EPX rendering plugin.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_render_epx_end(void)
{
	if (!mdp_render_epx_hostSrv)
		return MDP_ERR_OK;
	
	// Unregister the renderer.
	mdp_render_epx_hostSrv->renderer_unregister(&mdp, &mdp_render_t);
	
	// Plugin is shut down.
	return MDP_ERR_OK;
}


/**
 * T_mdp_render_epx_cpp: Blits the image to the screen, 2x size, EPX filter.
 * @param destScreen Pointer to the destination screen buffer.
 * @param mdScreen Pointer to the MD screen buffer.
 * @param destPitch Pitch of destScreen.
 * @param srcPitch Pitch of mdScreen.
 * @param width Width of the image.
 * @param height Height of the image.
 */
template<typename pixel>
static inline void T_mdp_render_epx_cpp(pixel *destScreen, pixel *mdScreen,
					int destPitch, int srcPitch,
					int width, int height)
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
			pixel L = *(SrcLine - 1);
			pixel C = *(SrcLine);
			pixel R = *(SrcLine + 1);
			if (L != R)
			{
				pixel U = *(SrcLine - srcPitch);
				pixel D = *(SrcLine + srcPitch);
				if (U != D)
				{
					*DstLine1++ = (U == L) ? U : C;
					*DstLine1++ = (R == U) ? R : C;
					*DstLine2++ = (L == D) ? L : C;
					*DstLine2++ = (D == R) ? D : C;
					SrcLine++;
					continue;
				}
			}
			*DstLine1++ = C; 
			*DstLine1++ = C; 
			*DstLine2++ = C; 
			*DstLine2++ = C; 
			SrcLine++;
		}
	}
}


int MDP_FNCALL mdp_render_epx_cpp(MDP_Render_Info_t *renderInfo)
{
	if (!renderInfo)
		return -MDP_ERR_RENDER_INVALID_RENDERINFO;;
	
	if (renderInfo->bpp == 16 || renderInfo->bpp == 15)
	{
		T_mdp_render_epx_cpp(
			    (uint16_t*)renderInfo->destScreen,
			    (uint16_t*)renderInfo->mdScreen,
			    renderInfo->destPitch, renderInfo->srcPitch,
			    renderInfo->width, renderInfo->height);
	}
	else //if (renderInfo->bpp == 32)
	{
		T_mdp_render_epx_cpp(
			    (uint32_t*)renderInfo->destScreen,
			    (uint32_t*)renderInfo->mdScreen,
			    renderInfo->destPitch, renderInfo->srcPitch,
			    renderInfo->width, renderInfo->height);
	}
	
	return MDP_ERR_OK;
}
