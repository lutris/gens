/***************************************************************************
 * MDP: Blargg's NTSC renderer.                                            *
 *                                                                         *
 * Copyright (c) 2006 by Shay Green                                        *
 * MDP version Copyright (c) 2008-2009 by David Korth                      *
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

#include "mdp_render_blargg_ntsc.h"
#include "mdp_render_blargg_ntsc_plugin.h"

// MDP includes.
#include "mdp/mdp_stdint.h"
#include "mdp/mdp_error.h"

// MDP Host Services.
static mdp_host_t *mdp_render_blargg_ntsc_host_srv = NULL;


/**
 * mdp_render_blargg_ntsc_init(): Initialize the Blargg NTSC rendering plugin.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_render_blargg_ntsc_init(mdp_host_t *host_srv)
{
	// Save the MDP Host Services pointer.
	mdp_render_blargg_ntsc_host_srv = host_srv;
	
	// Initialize the NTSC renderer.
	mdp_md_ntsc_init();
	
	// Register the renderer.
	return mdp_render_blargg_ntsc_host_srv->renderer_register(&mdp, &mdp_render);
}


/**
 * mdp_render_blargg_ntsc_end(): Shut down the Blargg NTSC rendering plugin.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_render_blargg_ntsc_end(void)
{
	if (!mdp_render_blargg_ntsc_host_srv)
		return MDP_ERR_OK;
	
	// Unregister the renderer.
	mdp_render_blargg_ntsc_host_srv->renderer_unregister(&mdp, &mdp_render);
	
	// Shut down the NTSC renderer.
	mdp_md_ntsc_end();
	
	// Plugin is shut down.
	return MDP_ERR_OK;
}
