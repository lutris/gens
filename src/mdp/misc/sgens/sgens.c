/***************************************************************************
 * MDP: Sonic Gens.                                                        *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * SGens Copyright (c) 2002 by LOst                                        *
 * MDP port Copyright (c) 2008-2009 by David Korth                         *
 * MDP port Copyright (c) 2009 by Phil Costin                              *
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

#include "sgens.h"
#include "sgens_plugin.h"
#include "sgens_window.h"

// MDP includes.
#include "mdp/mdp_cpuflags.h"
#include "mdp/mdp_error.h"
#include "mdp/mdp_event.h"

// MDP Host Services.
const mdp_host_t *sgens_host_srv = NULL;
static int sgens_menuItemID = -1;

// Current ROM type.
SGENS_ROM_TYPE sgens_current_rom_type = SGENS_ROM_TYPE_NONE;

static int MDP_FNCALL sgens_menu_handler(int menu_item_id);
static int MDP_FNCALL sgens_event_handler(int event_id, void *event_info);


/**
 * sgens_init(): Initialize the Sonic Gens plugin.
 * @return MDP error code.
 */
int MDP_FNCALL sgens_init(const mdp_host_t *host_srv)
{
	if (!host_srv)
		return -MDP_ERR_INVALID_PARAMETERS;
	
	// Save the MDP Host Services pointer.
	sgens_host_srv = host_srv;
	
#if defined(GENS_UI_GTK)
	static const int uiType = MDP_UI_GTK2;
#elif defined(GENS_UI_WIN32)
	static const int uiType = MDP_UI_WIN32;
#elif defined(GENS_OS_HAIKU)
	static const int uiType = MDP_UI_HAIKU;
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
	
	// Register the MDP event handlers.
	sgens_host_srv->event_register(&mdp, MDP_EVENT_OPEN_ROM, sgens_event_handler);
	sgens_host_srv->event_register(&mdp, MDP_EVENT_CLOSE_ROM, sgens_event_handler);
	
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
	if (sgens_menuItemID >= 0)
	{
		sgens_host_srv->menu_item_remove(&mdp, sgens_menuItemID);
		sgens_menuItemID = -1;
	}
	
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


/**
 * sgens_window_mdp_event_handler(): Handle an MDP event.
 * @param event_id MDP event ID.
 * @param event_info MDP event information.
 */
static int MDP_FNCALL sgens_event_handler(int event_id, void *event_info)
{
	switch (event_id)
	{
		case MDP_EVENT_OPEN_ROM:
		{
			// Check the ROM to make sure SGens can handle it.
			mdp_event_open_rom_t *open_rom = (mdp_event_open_rom_t*)(event_info);
			sgens_current_rom_type = sgens_get_ROM_type(open_rom->system_id);
			
			if (sgens_current_rom_type > SGENS_ROM_TYPE_UNSUPPORTED &&
			    sgens_current_rom_type < SGENS_ROM_TYPE_MAX)
			{
				// ROM type is supported.
				sgens_host_srv->event_register(&mdp, MDP_EVENT_PRE_FRAME, sgens_event_handler);
			}
			else if (sgens_current_rom_type < SGENS_ROM_TYPE_NONE ||
				 sgens_current_rom_type >= SGENS_ROM_TYPE_MAX)
			{
				// Out of range. Change to SGENS_ROM_TYPE_UNSUPPORTED.
				sgens_current_rom_type = SGENS_ROM_TYPE_UNSUPPORTED;
			}
			
			// Update the SGens window.
			sgens_window_update_rom_type();
			break;
		}
		case MDP_EVENT_CLOSE_ROM:
		{
			// If SGens is handling the ROM, unregister the PRE_FRAME event handler.
			sgens_host_srv->event_unregister(&mdp, MDP_EVENT_PRE_FRAME, sgens_event_handler);
			
			// ROM type is "None".
			sgens_current_rom_type = SGENS_ROM_TYPE_NONE;
			
			// Update the SGens window.
			sgens_window_update_rom_type();
			
			break;
		}
		case MDP_EVENT_PRE_FRAME:
			// Update the SGens window.
			sgens_window_update();
			break;
	}
	
	return MDP_ERR_OK;
}
