/***************************************************************************
 * Gens: [MDP] Scale3x renderer.                                           *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
 * Scale3x Copyright (c) 2001 by Andrea Mazzoleni                          *
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

#include "mdp_render_scale3x.h"
#include "mdp_render_scale3x_plugin.h"

// Scale3x frontend.
#include "scalebit_3x.h"

// MDP includes.
#include "mdp/mdp_cpuflags.h"
#include "mdp/mdp_error.h"

// MDP Host Services.
static mdp_host_t *mdp_render_scale3x_host_srv = NULL;


/**
 * mdp_render_scale3x_init(): Initialize the Scale3x rendering plugin.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_render_scale3x_init(mdp_host_t *host_srv)
{
	// Save the MDP Host Services pointer.
	mdp_render_scale3x_host_srv = host_srv;
	
	// Register the renderer.
	mdp_render_scale3x_host_srv->renderer_register(&mdp, &mdp_render);
	
	// Initialized.
	return MDP_ERR_OK;
}


/**
 * mdp_render_scale3x_end(): Shut down the Scale3x rendering plugin.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_render_scale3x_end(void)
{
	if (!mdp_render_scale3x_host_srv)
		return MDP_ERR_OK;
	
	// Unregister the renderer.
	mdp_render_scale3x_host_srv->renderer_unregister(&mdp, &mdp_render);
	
	// Plugin is shut down.
	return MDP_ERR_OK;
}


int MDP_FNCALL mdp_render_scale3x_cpp(mdp_render_info_t *render_info)
{
	if (!render_info)
		return -MDP_ERR_RENDER_INVALID_RENDERINFO;
	
	const unsigned int bytespp = (render_info->vmodeFlags & MDP_RENDER_VMODE_BPP) ? 4 : 2;
	
	scale3x(render_info->destScreen, render_info->destPitch,
		render_info->mdScreen, render_info->srcPitch,
		bytespp, render_info->width, render_info->height);
	
	return MDP_ERR_OK;
}
