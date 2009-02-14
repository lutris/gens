/***************************************************************************
 * Gens: [MDP] Sonic Gens.                                                 *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * SGens Copyright (c) 2002 by LOst                                        *
 * MDP port Copyright (c) 2008-2009 by David Korth                         *
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
#include <stdio.h>

#include "sgens.hpp"
#include "sgens_plugin.h"
#include "sgens_window.h"

// MDP includes.
#include "mdp/mdp_cpuflags.h"
#include "mdp/mdp_error.h"
#include "mdp/mdp_event.h"

// MDP Host Services.
MDP_Host_t *sgens_host_srv = NULL;
static int sgens_menuItemID = 0;


static int MDP_FNCALL sgens_menu_handler(int menu_item_id);


/**
 * sgens_init(): Initialize the Sonic Gens plugin.
 * @return MDP error code.
 */
int MDP_FNCALL sgens_init(MDP_Host_t *host_srv)
{
	// Save the MDP Host Services pointer.
	sgens_host_srv = host_srv;
	
#if defined(GENS_UI_GTK)
	static const int uiType = MDP_UI_GTK2;
#elif defined(GENS_UI_WIN32)
	static const int uiType = MDP_UI_WIN32;
#endif
	
	// Check the UI type.
	if (sgens_host_srv->val_get(MDP_VAL_UI) != uiType)
	{
		// Unsupported UI type.
		sgens_host_srv = NULL;
		return -MDP_ERR_UNSUPPORTED_UI;
	}
	
	// Create a menu item.
	sgens_menuItemID = sgens_host_srv->menu_item_add(&mdp, &sgens_menu_handler, 0, "&Sonic Gens");
	printf("Sonic Gens plugin initialized. Menu item ID: 0x%04X\n", sgens_menuItemID);
	
	// Initialized.
	return MDP_ERR_OK;
}


/**
 * sgens_end(): Shut down the Sonic Gens plugin.
 * @return MDP error code.
 */
int MDP_FNCALL sgens_end(void)
{
	if (!sgens_host_srv)
		return MDP_ERR_OK;
	
	// Make sure the window is closed.
	sgens_window_close();
	
	// Remove the menu item.
	sgens_host_srv->menu_item_remove(&mdp, sgens_menuItemID);
	
	// Plugin is shut down.
	return MDP_ERR_OK;
}


/**
 * sgens_menu_handler(): Menu handler function.
 * @param menu_item_id Menu item ID.
 * @return MDP error code.
 */
static int MDP_FNCALL sgens_menu_handler(int menu_item_id)
{
	if (menu_item_id != sgens_menuItemID)
		return -MDP_ERR_MENU_INVALID_MENUID;
	
	// Show the Sonic Gens window.
	sgens_window_show(sgens_host_srv->window_get_main());
	return MDP_ERR_OK;
}
