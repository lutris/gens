/***************************************************************************
 * Gens: [MDP] Scale4x renderer.                                           *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
 * Scale4x Copyright (c) 2001 by Andrea Mazzoleni                          *
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

#include "mdp_render_scale4x.h"
#include "mdp_render_scale4x_plugin.h"

// Scale4x frontend.
#include "scalebit_4x.h"
#include "scalebit_4x_mmx.h"

// MDP includes.
#include "mdp/mdp_cpuflags.h"
#include "mdp/mdp_error.h"

// MDP Host Services.
static MDP_Host_t *mdp_render_scale4x_hostSrv = NULL;


/**
 * mdp_render_scale4x_init(): Initialize the Scale4x rendering plugin.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_render_scale4x_init(MDP_Host_t *hostSrv)
{
	// Save the MDP Host Services pointer.
	mdp_render_scale4x_hostSrv = hostSrv;
	
	// Register the renderer.
	mdp_render_scale4x_hostSrv->renderer_register(&mdp, &mdp_render_t);
	
	// Initialized.
	return MDP_ERR_OK;
}


/**
 * mdp_render_scale4x_end(): Shut down the Scale4x rendering plugin.
 */
int MDP_FNCALL mdp_render_scale4x_end(void)
{
	if (!mdp_render_scale4x_hostSrv)
		return MDP_ERR_OK;
	
	// Unregister the renderer.
	mdp_render_scale4x_hostSrv->renderer_unregister(&mdp, &mdp_render_t);
	
	// Plugin is shut down.
	return MDP_ERR_OK;
}


void MDP_FNCALL mdp_render_scale4x_cpp(MDP_Render_Info_t *renderInfo)
{
	if (!renderInfo)
		return;

	const unsigned int bytespp = (renderInfo->bpp == 15 ? 2 : renderInfo->bpp / 8);
	
#if defined(__GNUC__) && defined(__i386__)
	if (renderInfo->cpuFlags & MDP_CPUFLAG_MMX)
	{
		scale4x_mmx(renderInfo->destScreen, renderInfo->destPitch,
			    renderInfo->mdScreen, renderInfo->srcPitch,
			    bytespp, renderInfo->width, renderInfo->height);
	}
	else
#endif /* defined(__GNUC__) && defined(__i386__) */
	{
		scale4x(renderInfo->destScreen, renderInfo->destPitch,
			renderInfo->mdScreen, renderInfo->srcPitch,
			bytespp, renderInfo->width, renderInfo->height);
	}
}
