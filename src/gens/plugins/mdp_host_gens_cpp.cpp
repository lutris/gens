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

// C++ includes
#include <algorithm>
#include <string>
#include <list>
using std::string;
using std::list;

// MDP includes.
#include "mdp_host_gens.h"
#include "mdp/mdp_error.h"

// Menus.
#include "ui/common/gens/gens_menu.h"
#include "emulator/g_main.hpp"
#include "gens/gens_window_sync.hpp"

// Plugin Manager.
#include "pluginmgr.hpp"


/**
 * mdp_host_renderer_register(): Register a render plugin.
 * @param plugin MDP_t requesting plugin registration.
 * @param renderer MDP_Render_t struct containing renderer information.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_host_renderer_register(struct MDP_t *plugin, MDP_Render_t *renderer)
{
	if (!plugin || !renderer)
		return 1;	// TODO: Return an MDP error code.
	
	// Check the render interface version.
	if (MDP_VERSION_MAJOR(renderer->interfaceVersion) !=
	    MDP_VERSION_MAJOR(MDP_RENDER_INTERFACE_VERSION))
	{
		// Incorrect major interface version.
		// TODO: Add to a list of "incompatible" plugins.
		return 2;	// TODO: Return an MDP error code.
	}
	
	// Check if a plugin with this tag already exists.
	string tag = renderer->tag;
	std::transform(tag.begin(), tag.end(), tag.begin(), ::tolower);
	mapRenderPlugin::iterator existingMDP = PluginMgr::tblRenderPlugins.find(tag);
	if (existingMDP != PluginMgr::tblRenderPlugins.end())
	{
		// Plugin with this tag already exists.
		// TODO: Show an error.
		return 3;	// TODO: Return an MDP error code.
	}
	
	// TODO: Check the minor version.
	// Probably not needed right now, but may be needed later.
	
	// Add the plugin to the list.
	PluginMgr::lstRenderPlugins.push_back(renderer);
	
	// Add the plugin tag to the map.
	list<MDP_Render_t*>::iterator lstIter = PluginMgr::lstRenderPlugins.end();
	lstIter--;
	PluginMgr::tblRenderPlugins.insert(pairRenderPlugin(tag, lstIter));
	
	// Render plugin added.
	return 0;
}


/**
 * mdp_host_renderer_unregister(): Unregister a render plugin.
 * @param plugin MDP_t requesting plugin unregistration.
 * @param renderer MDP_Render_t struct containing renderer information.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_host_renderer_unregister(struct MDP_t *plugin, MDP_Render_t *renderer)
{
	// TODO: Implement this function.
	return 0;
}


/**
 * mdp_host_menu_item_add(): Add a menu item.
 * @param plugin MDP_t requesting the menu item.
 * @param handler Function to handle menu item callbacks.
 * @param menu_id Menu to add the menu item to. (Currently ignored.)
 * @param text Initial menu item text.
 * @return Menu item ID, or MDP error code.
 */
int MDP_FNCALL mdp_host_menu_item_add(struct MDP_t *plugin, mdp_menu_handler_fn handler,
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


/**
 * mdp_host_menu_item_remove(): Remove a menu item.
 * @param plugin MDP_t requesting the menu item.
 * @param menu_item_id Menu item ID.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_host_menu_item_remove(struct MDP_t *plugin, int menu_item_id)
{
	return -MDP_ERR_FUNCTION_NOT_IMPLEMENTED;
}


/**
 * mdp_host_menu_item_set_text(): Set menu item text.
 * @param plugin MDP_t requesting the menu item.
 * @param menu_item_id Menu item ID.
 * @param text Text to set.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_host_menu_item_set_text(struct MDP_t *plugin, int menu_item_id, const char *text)
{
	return -MDP_ERR_FUNCTION_NOT_IMPLEMENTED;
}


/**
 * mdp_host_menu_item_get_text(): get menu item text.
 * @param plugin MDP_t requesting the menu item.
 * @param menu_item_id Menu item ID.
 * @param text_buf Buffer to copy the text to.
 * @param size Size of text_buf.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_host_menu_item_get_text(struct MDP_t *plugin, int menu_item_id, char *text_buf, int size)
{
	return -MDP_ERR_FUNCTION_NOT_IMPLEMENTED;
}


/**
 * mdp_host_menu_item_set_checked(): Set menu item "checked" state.
 * @param plugin MDP_t requesting the menu item.
 * @param menu_item_id Menu item ID.
 * @param checked "Checked" state.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_host_menu_item_set_checked(struct MDP_t *plugin, int menu_item_id, int checked)
{
	return -MDP_ERR_FUNCTION_NOT_IMPLEMENTED;
}


/**
 * mdp_host_menu_item_get_checked(): Get menu item "checked" state.
 * @param plugin MDP_t requesting the menu item.
 * @param menu_item_id Menu item ID.
 * @return 0 if not checked; 1 if checked; negative number for MDP error code.
 */
int MDP_FNCALL mdp_host_menu_item_get_checked(struct MDP_t *plugin, int menu_item_id)
{
	return -MDP_ERR_FUNCTION_NOT_IMPLEMENTED;
}
