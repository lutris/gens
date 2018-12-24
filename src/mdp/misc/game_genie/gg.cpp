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
#include <string.h>
#include <stdio.h>

// C++ includes.
#include <string>
using std::string;

// libgsft includes.
#include "libgsft/gsft_strlcpy.h"

#include "gg.hpp"
#include "gg_plugin.h"
#include "gg_window.hpp"
#include "gg_engine.hpp"

// MDP includes.
#include "mdp/mdp_stdint.h"
#include "mdp/mdp_cpuflags.h"
#include "mdp/mdp_error.h"
#include "mdp/mdp_event.h"
#include "mdp/mdp_constants.h"
#include "mdp/mdp_mem.h"

// MDP Host Services.
const mdp_host_t *gg_host_srv = NULL;

static int gg_menuItemID = -1;

// ROM size.
int gg_mdp_rom_md_size = 0;

static int MDP_FNCALL gg_menu_handler(int menu_item_id);
static int MDP_FNCALL gg_event_handler(int event_id, void *event_info);

// Directory registration.
static int gg_dir_id = -1;
static char gg_save_path[1024];
static int MDP_FNCALL gg_dir_get(int dir_id, char *out_buf, unsigned int size);
static int MDP_FNCALL gg_dir_set(int dir_id, const char *buf);

// Currently loaded ROM.
static string gg_loaded_rom;
MDP_SYSTEM_ID gg_system_id = MDP_SYSTEM_UNKNOWN;

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
int MDP_FNCALL gg_init(const mdp_host_t *host_srv)
{
	if (!host_srv)
		return -MDP_ERR_INVALID_PARAMETERS;
	
	// Save the MDP Host Services pointer.
	gg_host_srv = host_srv;
	
#if defined(GENS_UI_GTK)
	static const int uiType = MDP_UI_GTK2;
#elif defined(GENS_UI_WIN32)
	static const int uiType = MDP_UI_WIN32;
#elif defined(GENS_OS_HAIKU)
	static const int uiType = MDP_UI_HAIKU;
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
	
	// Set the Game Genie directory to the default save path initially.
	gg_host_srv->dir_get_default_save_path(gg_save_path, sizeof(gg_save_path));
	
	// Attempt to register a directory.
	// If this fails, gg_dir_id will be less than 0.
	gg_dir_id = gg_host_srv->dir_register(&mdp, "Patch Codes", gg_dir_get, gg_dir_set);
	
	// Register the event handler.
	gg_host_srv->event_register(&mdp, MDP_EVENT_OPEN_ROM, gg_event_handler);
	gg_host_srv->event_register(&mdp, MDP_EVENT_CLOSE_ROM, gg_event_handler);
	
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
	
	// If a directory was registered, unregister it.
	if (gg_dir_id >= 0)
	{
		gg_host_srv->dir_unregister(&mdp, gg_dir_id);
		gg_dir_id = -1;
	}
	
	// Remove the menu item.
	if (gg_menuItemID >= 0)
	{
		gg_host_srv->menu_item_remove(&mdp, gg_menuItemID);
		gg_menuItemID = -1;
	}
	
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
		
		// Save the ROM name and system ID.
		mdp_event_open_rom_t *openROM = (mdp_event_open_rom_t*)(event_info);
		gg_loaded_rom = string(openROM->rom_name);
		gg_system_id = (MDP_SYSTEM_ID)(openROM->system_id);
		
		// Get the ROM size.
		gg_mdp_rom_md_size = gg_host_srv->mem_size_get(MDP_MEM_MD_ROM);
		// TODO: Event handler for "ROM size changed".
		
		// Patch file is [save directory]/ROM_name.pat
		// Load the patch file.
		string full_path = string(gg_save_path) + "/" + gg_loaded_rom + string(GG_FILE_EXT);
		gg_file_load(full_path.c_str());
		
		// Set up the pre-frame event handler.
		if (gg_host_srv->event_register(&mdp, MDP_EVENT_PRE_FRAME, gg_engine_pre_frame) != MDP_ERR_OK)
		{
			// Error registering the pre-frame event handler.
			return -MDP_ERR_UNKNOWN;
		}
	}
	else if (event_id == MDP_EVENT_CLOSE_ROM)
	{
		// ROM closed.
		if (gg_loaded_rom.empty())
			return MDP_ERR_OK;
		
		// ROM name specified. Save the patch code file.
		
		// Patch file is [save directory]/ROM_name.pat
		// Save the patch file.
		string full_path = string(gg_save_path) + gg_loaded_rom + string(GG_FILE_EXT);
		gg_file_save(full_path.c_str());
		
		// Clear the loaded ROM name, system ID, and ROM size.
		gg_loaded_rom.clear();
		gg_system_id = MDP_SYSTEM_UNKNOWN;
		gg_mdp_rom_md_size = 0;
		
		// Clear all loaded codes.
		gg_code_list.clear();
		
		// Shut down the pre-frame event handler.
		gg_host_srv->event_unregister(&mdp, MDP_EVENT_PRE_FRAME, gg_engine_pre_frame);
	}
	
	return MDP_ERR_OK;
}


/**
 * gg_dir_get(): Get the Game Genie directory.
 * @param dir_id Directory ID.
 * @param out_buf Buffer to store the Game Genie directory in.
 * @param size Size of the buffer.
 * @return MDP error code.
 */
static int MDP_FNCALL gg_dir_get(int dir_id, char *out_buf, unsigned int size)
{
	if (dir_id != gg_dir_id)
		return -MDP_ERR_DIR_INVALID_DIRID;
	
	strlcpy(out_buf, gg_save_path, size);
	return MDP_ERR_OK;
}


/**
 * gg_dir_set(): Set the Game Genie directory.
 * @param dir_id Directory ID.
 * @param buf New directory.
 * @return MDP error code.
 */
static int MDP_FNCALL gg_dir_set(int dir_id, const char *buf)
{
	if (dir_id != gg_dir_id)
		return -MDP_ERR_DIR_INVALID_DIRID;
	
	strlcpy(gg_save_path, buf, sizeof(gg_save_path));
	return MDP_ERR_OK;
}
