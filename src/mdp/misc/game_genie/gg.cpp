/***************************************************************************
 * MDP: Game Genie.                                                        *
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

// C++ includes.
#include <string>
using std::string;

#include "gg.hpp"
#include "gg_plugin.h"
#include "gg_window.hpp"

// MDP includes.
#include "mdp/mdp_cpuflags.h"
#include "mdp/mdp_error.h"
#include "mdp/mdp_event.h"

// MDP Host Services.
mdp_host_t *gg_host_srv = NULL;

static int gg_menuItemID = 0;
static void *mdp_ptr_ram_md = NULL;

static int MDP_FNCALL gg_menu_handler(int menu_item_id);
static int MDP_FNCALL gg_event_handler(int event_id, void *event_info);

// Currently loaded ROM.
static string gg_loaded_rom;

// List of Game Genie codes.
#include "gg_code.h"
#include <list>
using std::list;
list<gg_code_t> gg_code_list;

// Patch code file handler.
#include "gg_file.hpp"


/**
 * gg_init(): Initialize the Game Genie plugin.
 * @return MDP error code.
 */
int MDP_FNCALL gg_init(mdp_host_t *host_srv)
{
	// Save the MDP Host Services pointer.
	gg_host_srv = host_srv;
	
#if defined(GENS_UI_GTK)
	static const int uiType = MDP_UI_GTK2;
#elif defined(GENS_UI_WIN32)
	static const int uiType = MDP_UI_WIN32;
#endif
	
	// Check the UI type.
	if (gg_host_srv->val_get(MDP_VAL_UI) != uiType)
	{
		// Unsupported UI type.
		gg_host_srv = NULL;
		return -MDP_ERR_UNSUPPORTED_UI;
	}
	
	// Create a menu item.
	gg_menuItemID = gg_host_srv->menu_item_add(&mdp, &gg_menu_handler, 0, "&Game Genie");
	printf("Game Genie plugin initialized. Menu item ID: 0x%04X\n", gg_menuItemID);
	
	// Register the event handler.
	gg_host_srv->event_register(&mdp, MDP_EVENT_OPEN_ROM, gg_event_handler);
	gg_host_srv->event_register(&mdp, MDP_EVENT_CLOSE_ROM, gg_event_handler);
	
	// Get the MD RAM.
	mdp_ptr_ram_md = gg_host_srv->ptr_ref(MDP_PTR_RAM_MD);
	
	// Initialized.
	return MDP_ERR_OK;
}


/**
 * gg_end(): Shut down the Game Genie plugin.
 * @return MDP error code.
 */
int MDP_FNCALL gg_end(void)
{
	if (!gg_host_srv)
		return MDP_ERR_OK;
	
	// Make sure the window is closed.
	gg_window_close();
	
	// Remove the menu item.
	gg_host_srv->menu_item_remove(&mdp, gg_menuItemID);
	
	// Unreference MD RAM.
	gg_host_srv->ptr_unref(MDP_PTR_RAM_MD);
	
	// Plugin is shut down.
	return MDP_ERR_OK;
}


/**
 * gg_menu_handler(): Menu handler function.
 * @param menu_item_id Menu item ID.
 * @return MDP error code.
 */
static int MDP_FNCALL gg_menu_handler(int menu_item_id)
{
	if (menu_item_id != gg_menuItemID)
		return -MDP_ERR_MENU_INVALID_MENUID;
	
	// Show the Game Genie window.
	gg_window_show(gg_host_srv->window_get_main());
	return MDP_ERR_OK;
}


static int MDP_FNCALL gg_event_handler(int event_id, void *event_info)
{
	if (event_id == MDP_EVENT_OPEN_ROM)
	{
		// ROM opened.
		
		// Save the ROM name.
		mdp_event_open_rom_t *openROM = (mdp_event_open_rom_t*)(event_info);
		gg_loaded_rom = string(openROM->rom_name);
		
		// Patch file is [save directory]/ROM_name.pat
		// TODO: Register a Game Genie-specific directory.
		char def_save_path[1024];
		gg_host_srv->directory_get_default_save_path(def_save_path, sizeof(def_save_path));
		
		// Load the patch file.
		string full_path = string(def_save_path) + gg_loaded_rom + string(GG_FILE_EXT);
		gg_file_load(full_path.c_str());
	}
	else if (event_id == MDP_EVENT_CLOSE_ROM)
	{
		// ROM closed.
		if (!gg_loaded_rom.empty())
		{
			// ROM name specified. Save the patch code file.
			
			// Patch file is [save directory]/ROM_name.pat
			// TODO: Register a Game Genie-specific directory.
			char def_save_path[1024];
			gg_host_srv->directory_get_default_save_path(def_save_path, sizeof(def_save_path));
			
			// Save the patch file.
			string full_path = string(def_save_path) + gg_loaded_rom + string(GG_FILE_EXT);
			gg_file_save(full_path.c_str());
			
			// Clear the loaded ROM name.
			gg_loaded_rom.clear();
		}
	}
	
	return MDP_ERR_OK;
}
