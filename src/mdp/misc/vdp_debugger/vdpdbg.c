/***************************************************************************
 * MDP: VDP Debugger.                                                      *
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

#include "vdpdbg.h"
#include "vdpdbg_plugin.h"
#include "vdpdbg_window.h"

// MDP includes.
#include "mdp/mdp_cpuflags.h"
#include "mdp/mdp_error.h"
#include "mdp/mdp_event.h"

// MDP Host Services.
const mdp_host_t *vdpdbg_host_srv = NULL;
static int vdpdbg_menuItemID = -1;

static int MDP_FNCALL vdpdbg_menu_handler(int menu_item_id);
static int MDP_FNCALL vdpdbg_event_handler(int event_id, void *event_info);


/**
 * vdpdbg_init(): Initialize the VDP Debugger plugin.
 * @return MDP error code.
 */
int MDP_FNCALL vdpdbg_init(const mdp_host_t *host_srv)
{
	if (!host_srv)
		return -MDP_ERR_INVALID_PARAMETERS;
	
	// Save the MDP Host Services pointer.
	vdpdbg_host_srv = host_srv;
	
#if defined(GENS_UI_GTK)
	static const int uiType = MDP_UI_GTK2;
#elif defined(GENS_UI_WIN32)
	static const int uiType = MDP_UI_WIN32;
#elif defined(GENS_OS_HAIKU)
	static const int uiType = MDP_UI_HAIKU;
#endif
	
	// Check the UI type.
	if (vdpdbg_host_srv->val_get(MDP_VAL_UI) != uiType)
	{
		// Unsupported UI type.
		vdpdbg_host_srv = NULL;
		return -MDP_ERR_UNSUPPORTED_UI;
	}
	
	// Create a menu item.
	vdpdbg_menuItemID = vdpdbg_host_srv->menu_item_add(&mdp, &vdpdbg_menu_handler, 0, "&VDP Debugger");
	
	// Register the MDP event handlers.
	vdpdbg_host_srv->event_register(&mdp, MDP_EVENT_OPEN_ROM, vdpdbg_event_handler);
	vdpdbg_host_srv->event_register(&mdp, MDP_EVENT_CLOSE_ROM, vdpdbg_event_handler);
	
	// Initialized.
	return MDP_ERR_OK;
}


/**
 * vdpdbg_end(): Shut down the VDP Debugger plugin.
 * @return MDP error code.
 */
int MDP_FNCALL vdpdbg_end(void)
{
	if (!vdpdbg_host_srv)
		return MDP_ERR_OK;
	
	// Make sure the window is closed.
	vdpdbg_window_close();
	
	// Remove the menu item.
	if (vdpdbg_menuItemID >= 0)
	{
		vdpdbg_host_srv->menu_item_remove(&mdp, vdpdbg_menuItemID);
		vdpdbg_menuItemID = -1;
	}
	
	// Unregister the MDP event handlers.
	vdpdbg_host_srv->event_unregister(&mdp, MDP_EVENT_OPEN_ROM, vdpdbg_event_handler);
	vdpdbg_host_srv->event_unregister(&mdp, MDP_EVENT_CLOSE_ROM, vdpdbg_event_handler);
	
	// Plugin is shut down.
	return MDP_ERR_OK;
}


/**
 * vdpdbg_menu_handler(): Menu handler function.
 * @param menu_item_id Menu item ID.
 * @return MDP error code.
 */
static int MDP_FNCALL vdpdbg_menu_handler(int menu_item_id)
{
	if (menu_item_id != vdpdbg_menuItemID)
		return -MDP_ERR_MENU_INVALID_MENUID;
	
	// Show the VDP Debugger window.
	vdpdbg_window_show(vdpdbg_host_srv->window_get_main());
	return MDP_ERR_OK;
}


/**
 * vdpdbg_window_mdp_event_handler(): Handle an MDP event.
 * @param event_id MDP event ID.
 * @param event_info MDP event information.
 */
static int MDP_FNCALL vdpdbg_event_handler(int event_id, void *event_info)
{
	switch (event_id)
	{
		case MDP_EVENT_OPEN_ROM:
			vdpdbg_host_srv->event_register(&mdp, MDP_EVENT_POST_FRAME, vdpdbg_event_handler);
			vdpdbg_window_update();
			break;
		
		case MDP_EVENT_CLOSE_ROM:
			vdpdbg_host_srv->event_unregister(&mdp, MDP_EVENT_POST_FRAME, vdpdbg_event_handler);
			vdpdbg_window_update();
			break;
		
		case MDP_EVENT_POST_FRAME:
			vdpdbg_window_update();
			break;
		
		default:
			break;
	}
	
	return MDP_ERR_OK;
}
