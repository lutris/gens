/***************************************************************************
 * Gens: [MDP] 2xSaI renderer.                                             *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
 * 2xSaI Copyright (c) by Derek Liauw Kie Fa and Robert J. Ohannessian     *
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

#include "mdp_render_2xsai.h"
#include "mdp_render_2xsai_plugin.h"

#include <stdint.h>
#include <string.h>

// MDP includes.
#include "mdp/mdp_cpuflags.h"
#include "mdp/mdp_error.h"

// x86 asm versions.
#ifdef GENS_X86_ASM
#include "mdp_render_2xsai_x86.h"
#endif /* GENS_X86_ASM */

// MDP Host Services.
static MDP_Host_t *mdp_render_2xsai_host_srv = NULL;


/**
 * mdp_render_2xsai_init(): Initialize the 2xSaI rendering plugin.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_render_2xsai_init(MDP_Host_t *host_srv)
{
	// Save the MDP Host Services pointer.
	mdp_render_2xsai_host_srv = host_srv;
	
	// Register the renderer.
	mdp_render_2xsai_host_srv->renderer_register(&mdp, &mdp_render_t);
	
	// Initialized.
	return MDP_ERR_OK;
}


/**
 * mdp_render_2xsai_end(): Shut down the 2xSaI rendering plugin.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_render_2xsai_end(void)
{
	if (!mdp_render_2xsai_host_srv)
		return MDP_ERR_OK;
	
	// Unregister the renderer.
	mdp_render_2xsai_host_srv->renderer_unregister(&mdp, &mdp_render_t);
	
	// Plugin is shut down.
	return MDP_ERR_OK;
}


/**
 * mdp_render_2xsai_cpp(): 2xSaI rendering function.
 * @param renderInfo Render information.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_render_2xsai_cpp(MDP_Render_Info_t *renderInfo)
{
	if (!renderInfo)
		return -MDP_ERR_RENDER_INVALID_RENDERINFO;
	
#ifdef GENS_X86_ASM
	if (renderInfo->cpuFlags & MDP_CPUFLAG_MMX)
	{
		mdp_render_2xsai_16_x86_mmx(
			    (uint16_t*)renderInfo->destScreen,
			    (uint16_t*)renderInfo->mdScreen,
			    renderInfo->destPitch, renderInfo->srcPitch,
			    renderInfo->width, renderInfo->height,
			    (renderInfo->bpp == 15));
	}
#else /* !GENS_X86_ASM */
	T_mdp_render_2xsai_cpp(
		    (uint16_t*)renderInfo->destScreen,
		    (uint16_t*)renderInfo->mdScreen,
		    renderInfo->destPitch, renderInfo->srcPitch,
		    renderInfo->width, renderInfo->height);
#endif /* GENS_X86_ASM */
	
	return MDP_ERR_OK;
}
