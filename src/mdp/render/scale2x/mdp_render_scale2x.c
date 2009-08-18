/***************************************************************************
 * MDP: Scale2x renderer.                                                  *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2009 by David Korth                                  *
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

// Scale2x frontend.
#include "scalebit_2x.h"

// Scale2x MMX frontend.
#if defined(GENS_X86_ASM) && defined(__GNUC__) && defined(__i386__)
#include "scalebit_2x_mmx.h"
#endif

// MDP includes.
#include "mdp/mdp_cpuflags.h"
#include "mdp/mdp_error.h"

// MDP Host Services.
static const mdp_host_t *mdp_render_scale2x_host_srv = NULL;


/**
 * mdp_render_scale2x_init(): Initialize the Scale2x rendering plugin.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_render_scale2x_init(const mdp_host_t *host_srv)
{
	if (!host_srv)
		return -MDP_ERR_INVALID_PARAMETERS;
	
	// Save the MDP Host Services pointer.
	mdp_render_scale2x_host_srv = host_srv;
	
	// Register the renderer.
	return mdp_render_scale2x_host_srv->renderer_register(&mdp, &mdp_render);
}


/**
 * mdp_render_scale2x_end(): Shut down the Scale2x rendering plugin.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_render_scale2x_end(void)
{
	if (!mdp_render_scale2x_host_srv)
		return MDP_ERR_OK;
	
	// Unregister the renderer.
	mdp_render_scale2x_host_srv->renderer_unregister(&mdp, &mdp_render);
	
	// Plugin is shut down.
	return MDP_ERR_OK;
}


int MDP_FNCALL mdp_render_scale2x_cpp(const mdp_render_info_t *render_info)
{
	if (!render_info)
		return -MDP_ERR_RENDER_INVALID_RENDERINFO;
	
	if (MDP_RENDER_VMODE_GET_SRC(render_info->vmodeFlags) !=
	    MDP_RENDER_VMODE_GET_DST(render_info->vmodeFlags))
	{
		// Renderer only supports identical src/dst modes.
		return -MDP_ERR_RENDER_UNSUPPORTED_VMODE;
	}
	
	const unsigned int bytespp = ((MDP_RENDER_VMODE_GET_SRC(render_info->vmodeFlags) == MDP_RENDER_VMODE_RGB_888) ? 4 : 2);
	
#if defined(GENS_X86_ASM) && defined(__GNUC__) && defined(__i386__)
	if (render_info->cpuFlags & MDP_CPUFLAG_X86_MMX)
	{
		scale2x_mmx(render_info->destScreen, render_info->destPitch,
			    render_info->mdScreen, render_info->srcPitch,
			    bytespp, render_info->width, render_info->height);
	}
	else
#endif /* defined(GENS_X86_ASM) && defined(__GNUC__) && defined(__i386__) */
	{
		scale2x(render_info->destScreen, render_info->destPitch,
			render_info->mdScreen, render_info->srcPitch,
			bytespp, render_info->width, render_info->height);
	}
	
	return MDP_ERR_OK;
}
