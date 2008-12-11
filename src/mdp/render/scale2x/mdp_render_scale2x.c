/***************************************************************************
 * Gens: [MDP] Scale2x renderer.                                           *
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

#include "mdp_render_scale2x.h"
#include "mdp_render_scale2x_plugin.h"
#include <string.h>
#include <stdint.h>

// Scale2x frontend.
#include "scalebit_2x.h"
#include "scalebit_2x_mmx.h"

// CPU flags
#include "mdp/mdp_cpuflags.h"

// MDP Host Services
static MDP_Host_t *mdp_render_scale2x_hostSrv = NULL;


/**
 * mdp_render_scale2x_init(): Initialize the Scale2x rendering plugin.
 */
void MDP_FNCALL mdp_render_scale2x_init(MDP_Host_t *hostSrv)
{
	// Save the MDP Host Services pointer.
	mdp_render_scale2x_hostSrv = hostSrv;
	
	// Register the renderer.
	mdp_render_scale2x_hostSrv->register_renderer(&mdp, &mdp_render_t);
}


/**
 * mdp_render_scale2x_end(): Shut down the Scale2x rendering plugin.
 */
void MDP_FNCALL mdp_render_scale2x_end(void)
{
	if (mdp_render_scale2x_hostSrv)
	{
		// Unregister the renderer.
		mdp_render_scale2x_hostSrv->unregister_renderer(&mdp, &mdp_render_t);
	}
}


void MDP_FNCALL mdp_render_scale2x_cpp(MDP_Render_Info_t *renderInfo)
{
	if (!renderInfo)
		return;

	const unsigned int bytespp = (renderInfo->bpp == 15 ? 2 : renderInfo->bpp / 8);
	
#if defined(__GNUC__) && defined(__i386__)
	if (renderInfo->cpuFlags & MDP_CPUFLAG_MMX)
	{
		scale2x_mmx(renderInfo->destScreen, renderInfo->destPitch,
			    renderInfo->mdScreen, renderInfo->srcPitch,
			    bytespp, renderInfo->width, renderInfo->height);
	}
	else
#endif /* defined(__GNUC__) && defined(__i386__) */
	{
		scale2x(renderInfo->destScreen, renderInfo->destPitch,
			renderInfo->mdScreen, renderInfo->srcPitch,
			bytespp, renderInfo->width, renderInfo->height);
	}
}
