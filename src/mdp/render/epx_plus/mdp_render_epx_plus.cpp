/***************************************************************************
 * Gens: [MDP] EPX Plus renderer.                                          *
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

#include "mdp_render_epx_plus.hpp"
#include "mdp_render_epx_plus_plugin.h"
#include <string.h>
#include <stdint.h>

// Mask constants
#define MASK_DIV2_15		((uint16_t)(0x3DEF))
#define MASK_DIV2_16		((uint16_t)(0x7BEF))
#define MASK_DIV2_15_ASM	((uint32_t)(0x3DEF3DEF))
#define MASK_DIV2_16_ASM	((uint32_t)(0x7BEF7BEF))
#define MASK_DIV2_32		((uint32_t)(0x7F7F7F7F))

#define BLEND(a, b, mask) ((((a) >> 1) & mask) + (((b) >> 1) & mask))

// MDP Host Services
static MDP_Host_t *mdp_render_epx_plus_hostSrv = NULL;


/**
 * mdp_render_epx_plus_init(): Initialize the EPX Plus rendering plugin.
 */
void MDP_FNCALL mdp_render_epx_plus_init(MDP_Host_t *hostSrv)
{
	// Save the MDP Host Services pointer.
	mdp_render_epx_plus_hostSrv = hostSrv;
	
	// Register the renderer.
	mdp_render_epx_plus_hostSrv->renderer_register(&mdp, &mdp_render_t);
}


/**
 * mdp_render_epx_plus_end(): Shut down the EPX Plus rendering plugin.
 */
void MDP_FNCALL mdp_render_epx_plus_end(void)
{
	if (!mdp_render_epx_plus_hostSrv)
		return;
	
	// Unregister the renderer.
	mdp_render_epx_plus_hostSrv->renderer_unregister(&mdp, &mdp_render_t);
}


/**
 * T_mdp_render_epx_plus_cpp: Blits the image to the screen, 2x size, EPX Plus filter.
 * @param destScreen Pointer to the destination screen buffer.
 * @param mdScreen Pointer to the MD screen buffer.
 * @param destPitch Pitch of destScreen.
 * @param srcPitch Pitch of mdScreen.
 * @param width Width of the image.
 * @param height Height of the image.
 * @param mask Mask for BLEND().
 */
template<typename pixel>
static inline void T_mdp_render_epx_plus_cpp(pixel *destScreen, pixel *mdScreen,
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
		
		pixel UL = *(SrcLine - srcPitch - 1);
		pixel U  = *(SrcLine - srcPitch);
		pixel UR = *(SrcLine - srcPitch + 1);
		pixel LL = *(SrcLine - 2);
		pixel L  = *(SrcLine - 1);
		pixel C  = *(SrcLine);
		pixel R  = *(SrcLine + 1);
		pixel RR = *(SrcLine + 2);
		pixel DL = *(SrcLine + srcPitch - 1);
		pixel D  = *(SrcLine + srcPitch);
		pixel DR = *(SrcLine + srcPitch + 1);
		
		for (int x = 0; x < width; x++)
		{
			UL = U, U = UR;
			LL = L, L = C, C = R, R = RR;
			DL = D, D = DR;
			
			UR = *(SrcLine - srcPitch + 1);
			RR = *(SrcLine + 2);
			DR = *(SrcLine + srcPitch + 1);
			
			pixel UU, DD;
			if (L != R && U != D)
			{
				UU = *(SrcLine - (2 * srcPitch));
				DD = *(SrcLine + (2 * srcPitch));
			}
			
			*DstLine1++ = (L != R && U != D && U == L && (UR == R || DL == D || R != RR || D != DD))
					? ((UR == DL && (L == LL || U == UU))      ? BLEND(C, U, mask) : U)
					: ((L == U && C == UR && L != C && L == D) ? BLEND(C, L, mask) : C);
			*DstLine1++ = (R != L && U != D && U == R && (UL == L || DR == D || L != LL || D != DD))
					? ((UL == DR && (R == RR || U == UU))      ? BLEND(C, U, mask) : U)
					: ((R == U && C == UL && R != C && R == D) ? BLEND(C, R, mask) : C);
			
			*DstLine2++ = (L != R && D != U && D == L && (DR == R || UL == U || R != RR || U != UU))
					? ((DR == UL && (L == LL || D == DD))      ? BLEND(C, D, mask) : D)
					: ((L == D && C == DR && L != C && L == U) ? BLEND(C, L, mask) : C);
			*DstLine2++ = (R != L && D != U && D == R && (DL == L || UR == U || L != LL || U != UU))
					? ((DL == UR && (R == RR || D == DD))      ? BLEND(C, D, mask) : D)
					: ((R == D && C == DL && R != C && R == U) ? BLEND(C, R, mask) : C);
			
			SrcLine++;
		}
	}
}


void MDP_FNCALL mdp_render_epx_plus_cpp(MDP_Render_Info_t *renderInfo)
{
	if (!renderInfo)
		return;
	
	if (renderInfo->bpp == 16 || renderInfo->bpp == 15)
	{
		T_mdp_render_epx_plus_cpp(
			    (uint16_t*)renderInfo->destScreen,
			    (uint16_t*)renderInfo->mdScreen,
			    renderInfo->destPitch, renderInfo->srcPitch,
			    renderInfo->width, renderInfo->height,
			    (renderInfo->bpp == 15 ? MASK_DIV2_15 : MASK_DIV2_16));
	}
	else //if (renderInfo->bpp == 32)
	{
		T_mdp_render_epx_plus_cpp(
			    (uint32_t*)renderInfo->destScreen,
			    (uint32_t*)renderInfo->mdScreen,
			    renderInfo->destPitch, renderInfo->srcPitch,
			    renderInfo->width, renderInfo->height,
			    MASK_DIV2_32);
	}
}
