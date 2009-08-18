/***************************************************************************
 * MDP: hq4x renderer.                                                     *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2009 by David Korth                                  *
 * hq4x Copyright (c) 2003 by Maxim Stepin                                 *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU Lesser General Public License as published   *
 * by the Free Software Foundation; either version 2.1 of the License, or  *
 * (at your option) any later version.                                     *
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

#include "mdp_render_hq4x.h"
#include "mdp_render_hq4x_plugin.h"
#include "mdp_render_hq4x_RGB.h"

// C includes.
#include <stdlib.h>
#include <string.h>

// MDP includes.
#include "mdp/mdp_stdint.h"
#include "mdp/mdp_cpuflags.h"
#include "mdp/mdp_error.h"

// x86 asm versions.
#ifdef GENS_X86_ASM
#include "mdp_render_hq4x_x86.h"
#endif /* GENS_X86_ASM */

// MDP Host Services.
static const mdp_host_t *mdp_render_hq4x_host_srv;

// RGB lookup tables.
uint32_t *mdp_render_hq4x_RGB16to32 = NULL;
uint32_t *mdp_render_hq4x_RGB16toYUV = NULL;


// TODO: Proper 15-bit color support.
// It "works' right now in 15-bit, but the output is different than with 16-bit.
// My first attempt at 15-bit support resulted in massive failure. :(


/**
 * mdp_render_hq4x_end(): Initialize the hq4x plugin. 
 * @return MDP error code.
 */
int MDP_FNCALL mdp_render_hq4x_init(const mdp_host_t *host_srv)
{
	if (!host_srv)
		return -MDP_ERR_INVALID_PARAMETERS;
	
	// Save the MDP Host Services pointer.
	mdp_render_hq4x_host_srv = host_srv;
	
	// Register the renderer.
	return mdp_render_hq4x_host_srv->renderer_register(&mdp, &mdp_render);
}


/**
 * mdp_render_hq4x_end(): Shut down the hq4x plugin.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_render_hq4x_end(void)
{
	if (!mdp_render_hq4x_host_srv)
		return MDP_ERR_OK;
	
	// Unregister the renderer.
	mdp_render_hq4x_host_srv->renderer_unregister(&mdp, &mdp_render);
	
	// If the RGB16to32 table was allocated, free it.
	free(mdp_render_hq4x_RGB16to32);
	mdp_render_hq4x_RGB16to32 = NULL;
	
	// If the RGB16toYUV table was allocated, free it.
	free(mdp_render_hq4x_RGB16toYUV);
	mdp_render_hq4x_RGB16toYUV = NULL;
	
	// Plugin is shut down.
	return MDP_ERR_OK;
}


/**
 * mdp_render_hq4x_cpp(): hq4x rendering function.
 * @param render_info Render information.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_render_hq4x_cpp(const mdp_render_info_t *render_info)
{
	if (!render_info)
		return -MDP_ERR_RENDER_INVALID_RENDERINFO;
	
	if (MDP_RENDER_VMODE_GET_SRC(render_info->vmodeFlags) !=
	    MDP_RENDER_VMODE_GET_DST(render_info->vmodeFlags))
	{
		// Renderer only supports identical src/dst modes.
		return -MDP_ERR_RENDER_UNSUPPORTED_VMODE;
	}
	
	int mode565;
	switch (MDP_RENDER_VMODE_GET_SRC(render_info->vmodeFlags))
	{
		case MDP_RENDER_VMODE_RGB_555:
			mode565 = 0;
			break;
		case MDP_RENDER_VMODE_RGB_565:
			mode565 = 1;
			break;
		default:
			// Unsupported video mode.
			return -MDP_ERR_RENDER_UNSUPPORTED_VMODE;
	}
	
	// Make sure the lookup tables are initialized.
	if (!mdp_render_hq4x_RGB16to32)
		mdp_render_hq4x_RGB16to32 = mdp_render_hq4x_build_RGB16to32();
	if (!mdp_render_hq4x_RGB16toYUV)
		mdp_render_hq4x_RGB16toYUV = mdp_render_hq4x_build_RGB16toYUV();
	
	if (render_info->cpuFlags & MDP_CPUFLAG_X86_MMX)
	{
		mdp_render_hq4x_16_x86_mmx(
			    (uint16_t*)render_info->destScreen,
			    (uint16_t*)render_info->mdScreen,
			    render_info->destPitch, render_info->srcPitch,
			    render_info->width, render_info->height,
			    mode565);
	}
	
	return MDP_ERR_OK;
}
