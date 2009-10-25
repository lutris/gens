/***************************************************************************
 * MDP: VDP Layer Options.                                                 *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2009 by David Korth                                  *
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

#include <stdio.h>

#include "vlopt.h"
#include "vlopt_plugin.h"
#include "vlopt_window.h"

// MDP includes.
#include "mdp/mdp_cpuflags.h"
#include "mdp/mdp_error.h"
#include "mdp/mdp_event.h"

// MDP Host Services.
const mdp_host_t *vlopt_host_srv = NULL;
static int vlopt_menuItemID = -1;


static int MDP_FNCALL vlopt_menu_handler(int menu_item_id);


/**
 * vlopt_init(): Initialize the VDP Layer Options plugin.
 * @return MDP error code.
 */
int MDP_FNCALL vlopt_init(const mdp_host_t *host_srv)
{
	if (!host_srv)
		return -MDP_ERR_INVALID_PARAMETERS;
	
	// Save the MDP Host Services pointer.
	vlopt_host_srv = host_srv;
	
#if defined(GENS_UI_GTK)
	static const int uiType = MDP_UI_GTK2;
#elif defined(GENS_UI_WIN32)
	static const int uiType = MDP_UI_WIN32;
#elif defined(GENS_OS_HAIKU)
	static const int uiType = MDP_UI_HAIKU;
#endif
	
	// Check the UI type.
	if (vlopt_host_srv->val_get(MDP_VAL_UI) != uiType)
	{
		// Unsupported UI type.
		vlopt_host_srv = NULL;
		return -MDP_ERR_UNSUPPORTED_UI;
	}
	
	// Check if MDP_VAL_VDP_LAYER_OPTIONS is supported.
	int vdp_layer_options = vlopt_host_srv->val_get(MDP_VAL_VDP_LAYER_OPTIONS);
	if (vdp_layer_options < 0)
	{
		// MDP_VAL_VDP_LAYER_OPTIONS isn't supported.
		return vdp_layer_options;
	}
	
	// Create a menu item.
	vlopt_menuItemID = vlopt_host_srv->menu_item_add(&mdp, &vlopt_menu_handler, 0, "VDP &Layer Options");
	
	// Initialized.
	return MDP_ERR_OK;
}


/**
 * vlopt_end(): Shut down the VDP Layer Options plugin.
 * @return MDP error code.
 */
int MDP_FNCALL vlopt_end(void)
{
	if (!vlopt_host_srv)
		return MDP_ERR_OK;
	
	// Make sure the window is closed.
	vlopt_window_close();
	
	// Remove the menu item.
	if (vlopt_menuItemID >= 0)
	{
		vlopt_host_srv->menu_item_remove(&mdp, vlopt_menuItemID);
		vlopt_menuItemID = -1;
	}
	
	// Plugin is shut down.
	return MDP_ERR_OK;
}


/**
 * vlopt_menu_handler(): Menu handler function.
 * @param menu_item_id Menu item ID.
 * @return MDP error code.
 */
static int MDP_FNCALL vlopt_menu_handler(int menu_item_id)
{
	if (menu_item_id != vlopt_menuItemID)
		return -MDP_ERR_MENU_INVALID_MENUID;
	
	// Show the VDP Layer Options window.
	vlopt_window_show(vlopt_host_srv->window_get_main());
	return MDP_ERR_OK;
}
