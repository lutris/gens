/***************************************************************************
 * MDP: IPS Patcher.                                                       *
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

// C includes.
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "ips.h"
#include "ips_plugin.h"

// MDP includes.
#include "mdp/mdp_cpuflags.h"
#include "mdp/mdp_error.h"
#include "mdp/mdp_event.h"
#include "mdp/mdp_constants.h"

// MDP Host Services.
mdp_host_t *ips_host_srv = NULL;

static int ips_menuItemID = 0;

static int MDP_FNCALL ips_menu_handler(int menu_item_id);
static int MDP_FNCALL ips_event_handler(int event_id, void *event_info);


/**
 * ips_init(): Initialize the IPS Patcher plugin.
 * @return MDP error code.
 */
int MDP_FNCALL ips_init(mdp_host_t *host_srv)
{
	// Save the MDP Host Services pointer.
	ips_host_srv = host_srv;
	
#if defined(GENS_UI_GTK)
	static const int uiType = MDP_UI_GTK2;
#elif defined(GENS_UI_WIN32)
	static const int uiType = MDP_UI_WIN32;
#endif
	
	// Check the UI type.
	if (ips_host_srv->val_get(MDP_VAL_UI) != uiType)
	{
		// Unsupported UI type.
		ips_host_srv = NULL;
		return -MDP_ERR_UNSUPPORTED_UI;
	}
	
	// Create a menu item.
	ips_menuItemID = ips_host_srv->menu_item_add(&mdp, &ips_menu_handler, 0, "&IPS Patcher");
	
	// Register the event handler. (TODO)
	//ips_host_srv->event_register(&mdp, MDP_EVENT_OPEN_ROM, ips_event_handler);
	//ips_host_srv->event_register(&mdp, MDP_EVENT_CLOSE_ROM, ips_event_handler);
	
	// Initialized.
	return MDP_ERR_OK;
}


/**
 * ips_end(): Shut down the IPS Patcher plugin.
 * @return MDP error code.
 */
int MDP_FNCALL ips_end(void)
{
	if (!ips_host_srv)
		return MDP_ERR_OK;
	
	// Remove the menu item.
	ips_host_srv->menu_item_remove(&mdp, ips_menuItemID);
	
	// Plugin is shut down.
	return MDP_ERR_OK;
}


/**
 * ips_menu_handler(): Menu handler function.
 * @param menu_item_id Menu item ID.
 * @return MDP error code.
 */
static int MDP_FNCALL ips_menu_handler(int menu_item_id)
{
	if (menu_item_id != ips_menuItemID)
		return -MDP_ERR_MENU_INVALID_MENUID;
	
	// Show the IPS Patcher window. (TODO)
	//ips_window_show(ips_host_srv->window_get_main());
	return MDP_ERR_OK;
}


static int MDP_FNCALL ips_event_handler(int event_id, void *event_info)
{
	// TODO
	return MDP_ERR_OK;
}
