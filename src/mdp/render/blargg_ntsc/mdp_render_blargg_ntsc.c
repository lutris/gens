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
#include "ntsc_window.h"
#include "md_ntsc.hpp"

// MDP includes.
#include "mdp/mdp_stdint.h"
#include "mdp/mdp_error.h"

// MDP Host Services.
mdp_host_t *ntsc_host_srv = NULL;
static int ntsc_menuItemID = 0;

static int MDP_FNCALL ntsc_menu_handler(int menu_item_id);


/**
 * mdp_render_blargg_ntsc_init(): Initialize the Blargg NTSC rendering plugin.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_render_blargg_ntsc_init(mdp_host_t *host_srv)
{
	// Save the MDP Host Services pointer.
	ntsc_host_srv = host_srv;
	
	// Initialize the NTSC renderer.
	mdp_md_ntsc_init();
	
	// Register the renderer.
	int rval = ntsc_host_srv->renderer_register(&mdp, &mdp_render);
	if (rval != MDP_ERR_OK)
		return rval;
	
	// Create a menu item.
	ntsc_menuItemID = ntsc_host_srv->menu_item_add(&mdp, &ntsc_menu_handler, 0, "Blargg's &NTSC Filter");
	if (ntsc_menuItemID < 0)
	{
		// Error creating the menu item.
		ntsc_menuItemID = 0;
	}
	
	// Plugin has been initialized.
	return MDP_ERR_OK;
}


/**
 * mdp_render_blargg_ntsc_end(): Shut down the Blargg NTSC rendering plugin.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_render_blargg_ntsc_end(void)
{
	if (!ntsc_host_srv)
		return MDP_ERR_OK;
	
	// Remove the menu item.
	if (ntsc_menuItemID != 0)
	{
		ntsc_host_srv->menu_item_remove(&mdp, ntsc_menuItemID);
		ntsc_menuItemID = 0;
	}
	
	// Unregister the renderer.
	ntsc_host_srv->renderer_unregister(&mdp, &mdp_render);
	
	// Shut down the NTSC renderer.
	mdp_md_ntsc_end();
	
	// Plugin is shut down.
	return MDP_ERR_OK;
}


/**
 * ntsc_menu_handler(): Menu handler function.
 * @param menu_item_id Menu item ID.
 * @return MDP error code.
 */
static int MDP_FNCALL ntsc_menu_handler(int menu_item_id)
{
	if (menu_item_id != ntsc_menuItemID)
		return -MDP_ERR_MENU_INVALID_MENUID;
	
	// Show the NTSC Plugin Options window.
	ntsc_window_show(ntsc_host_srv->window_get_main());
	return MDP_ERR_OK;
}
