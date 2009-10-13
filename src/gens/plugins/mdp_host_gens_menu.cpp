/***************************************************************************
 * Gens: MDP: Mega Drive Plugin - Host Services. (C++ Functions)           *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
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

#include "mdp_host_gens_menu.hpp"

// C includes.
#include <string.h>

// C++ includes.
#include <algorithm>
#include <string>
#include <list>
using std::string;
using std::list;

// libgsft includes.
#include "libgsft/gsft_strlcpy.h"

// MDP includes.
#include "mdp/mdp_error.h"

// Menus.
#include "ui/common/gens/gens_menu.h"
#include "emulator/g_main.hpp"
#include "gens/gens_window_sync.hpp"

// Plugin Manager.
#include "pluginmgr.hpp"


/**
 * mdp_host_menu_item_add(): Add a menu item.
 * @param plugin mdp_t requesting the menu item.
 * @param handler Function to handle menu item callbacks.
 * @param menu_id Menu to add the menu item to. (Currently ignored.)
 * @param text Initial menu item text.
 * @return Menu item ID, or MDP error code.
 */
int MDP_FNCALL mdp_host_menu_item_add(mdp_t *plugin, mdp_menu_handler_fn handler,
				      int menu_id, const char *text)
{
	// Check to see what the largest menu ID is.
	uint16_t menuItemID;
	
	// TODO: Allow adding to menus other than the Plugins menu.
	
	if (PluginMgr::lstMenuItems.size() == 0)
	{
		// No menu items.
		menuItemID = IDM_PLUGINS_MENU + 1;
	}
	else
	{
		// Get the menu ID of the last menu item, and add one.
		menuItemID = PluginMgr::lstMenuItems.back().id + 1;
		do
		{
			mapMenuItems::iterator curMenuItem = PluginMgr::tblMenuItems.find(menuItemID);
			if (curMenuItem == PluginMgr::tblMenuItems.end())
				 break;
			menuItemID++;
		} while (menuItemID < IDM_PLUGINS_MANAGER);
		
		if (menuItemID >= IDM_PLUGINS_MANAGER)
		{
			// Out of menu item IDs.
			return -MDP_ERR_MENU_TOO_MANY_ITEMS;
		}
	}
	
	// Add the menu item.
	mdpMenuItem_t menuItem;
	menuItem.id = menuItemID;
	menuItem.handler = handler;
	menuItem.owner = plugin;
	
	// Set the menu item text.
	if (text)
		menuItem.text = string(text);
	
	// Default attributes.
	menuItem.checked = false;
	
	// Add the menu item to the list.
	PluginMgr::lstMenuItems.push_back(menuItem);
	
	// Add the menu item ID to the map.
	list<mdpMenuItem_t>::iterator lstIter = PluginMgr::lstMenuItems.end();
	lstIter--;
	PluginMgr::tblMenuItems.insert(pairMenuItems(menuItemID, lstIter));
	
	// If Gens is running, synchronize the Plugins Menu.
	if (is_gens_running())
		Sync_Gens_Window_PluginsMenu();
	
	// Return the menu item ID.
	return menuItemID;
}


// Typedef for the menu item list iterator.
typedef list<mdpMenuItem_t>::iterator menuIter;


/**
 * getMenuItemIter(): Get a menu item iterator.
 * @param plugin Plugin that requested access to the menu item.
 * @param menu_item_id Menu item ID.
 * @param lstIter_ret List iterator. (Output)
 * @return True if found; false if not found or not owned by the plugin.
 */
static inline bool getMenuItemIter(mdp_t *plugin, int menu_item_id, menuIter& lstIter_ret)
{
	// Search for the menu item.
	mapMenuItems::iterator curMenuItem = PluginMgr::tblMenuItems.find(menu_item_id);
	if (curMenuItem == PluginMgr::tblMenuItems.end())
	{
		// Menu item not found.
		return false;
	}
	
	// Get the list iterator.
	lstIter_ret = (*curMenuItem).second;
	
	// Check if the menu item is owned by this plugin.
	if ((*lstIter_ret).owner != plugin)
	{
		// Not owned by the plugin.
		return false;
	}
	
	// Menu item is owned by the plugin.
	return true;
}


/**
 * mdp_host_menu_item_remove(): Remove a menu item.
 * @param plugin mdp_t requesting the menu item.
 * @param menu_item_id Menu item ID.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_host_menu_item_remove(mdp_t *plugin, int menu_item_id)
{
	menuIter lstIter;
	if (!getMenuItemIter(plugin, menu_item_id, lstIter))
	{
		// Menu item not found or not owned by this plugin.
		return -MDP_ERR_MENU_INVALID_MENUID;
	}
	
	// Remove the menu item.
	PluginMgr::tblMenuItems.erase(menu_item_id);
	PluginMgr::lstMenuItems.erase(lstIter);
	
	// If Gens is running, synchronize the Plugins Menu.
	if (is_gens_running())
		Sync_Gens_Window_PluginsMenu();
	
	// Menu item removed.
	return MDP_ERR_OK;
}


/**
 * mdp_host_menu_item_set_text(): Set menu item text.
 * @param plugin mdp_t requesting the menu item.
 * @param menu_item_id Menu item ID.
 * @param text Text to set.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_host_menu_item_set_text(mdp_t *plugin, int menu_item_id, const char *text)
{
	menuIter lstIter;
	if (!getMenuItemIter(plugin, menu_item_id, lstIter))
	{
		// Menu item not found or not owned by this plugin.
		return -MDP_ERR_MENU_INVALID_MENUID;
	}
	
	// Set the menu item text.
	if (text)
		(*lstIter).text = string(text);
	else
		(*lstIter).text.clear();
	
	// If Gens is running, synchronize the Plugins Menu.
	// TODO: Optimize this so that only the specific menu item is updated.
	if (is_gens_running())
		Sync_Gens_Window_PluginsMenu();
	
	// Menu item removed.
	return MDP_ERR_OK;
}


/**
 * mdp_host_menu_item_get_text(): get menu item text.
 * @param plugin mdp_t requesting the menu item.
 * @param menu_item_id Menu item ID.
 * @param text_buf Buffer to copy the text to.
 * @param size Size of text_buf.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_host_menu_item_get_text(mdp_t *plugin, int menu_item_id,
					   char *text_buf, unsigned int size)
{
	menuIter lstIter;
	if (!getMenuItemIter(plugin, menu_item_id, lstIter))
	{
		// Menu item not found or not owned by this plugin.
		return -MDP_ERR_MENU_INVALID_MENUID;
	}
	
	// Get the menu item text.
	// TODO: Return an error code if the buffer size isn't large enough.
	strlcpy(text_buf, (*lstIter).text.c_str(), size);
	
	// Menu item text copied.
	return MDP_ERR_OK;
}


/**
 * mdp_host_menu_item_set_checked(): Set menu item "checked" state.
 * @param plugin mdp_t requesting the menu item.
 * @param menu_item_id Menu item ID.
 * @param checked "Checked" state.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_host_menu_item_set_checked(mdp_t *plugin, int menu_item_id, int checked)
{
	menuIter lstIter;
	if (!getMenuItemIter(plugin, menu_item_id, lstIter))
	{
		// Menu item not found or not owned by this plugin.
		return -MDP_ERR_MENU_INVALID_MENUID;
	}
	
	// Set the menu item "checked" state.
	(*lstIter).checked = (checked ? 1 : 0);
	
	// If Gens is running, synchronize the Plugins Menu.
	// TODO: Optimize this so that only the specific menu item is updated.
	if (is_gens_running())
		Sync_Gens_Window_PluginsMenu();
	
	// Menu item removed.
	return MDP_ERR_OK;
}


/**
 * mdp_host_menu_item_get_checked(): Get menu item "checked" state.
 * @param plugin mdp_t requesting the menu item.
 * @param menu_item_id Menu item ID.
 * @return 0 if not checked; 1 if checked; negative number for MDP error code.
 */
int MDP_FNCALL mdp_host_menu_item_get_checked(mdp_t *plugin, int menu_item_id)
{
	menuIter lstIter;
	if (!getMenuItemIter(plugin, menu_item_id, lstIter))
	{
		// Menu item not found or not owned by this plugin.
		return -MDP_ERR_MENU_INVALID_MENUID;
	}
	
	// Get the menu item "checked" state.
	return ((*lstIter).checked ? 1 : 0);
}
