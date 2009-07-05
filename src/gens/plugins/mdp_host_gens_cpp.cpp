/***************************************************************************
 * Gens: MDP: Mega Drive Plugin - Host Services. (C++ Functions)           *
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

// C++ includes.
#include <string>
#include <list>
using std::string;
using std::list;

#include "mdp_host_gens_cpp.hpp"

// MDP includes.
#include "mdp/mdp_error.h"

// Plugin Manager and Render Manager.
#include "pluginmgr.hpp"
#include "rendermgr.hpp"

// Win32 functions.
#ifdef _WIN32
#include "mdp_host_gens_win32.h"
#endif


/**
 * mdp_host_renderer_register(): Register a render plugin.
 * @param plugin mdp_t requesting plugin registration.
 * @param renderer mdp_render_t struct containing renderer information.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_host_renderer_register(mdp_t *plugin, mdp_render_t *renderer)
{
	// This is simply a wrapper around RenderMgr::addRenderPlugin().
	// TODO: Record the plugin that registered the renderer.
	
	return RenderMgr::addRenderPlugin(renderer);
}


/**
 * mdp_host_renderer_unregister(): Unregister a render plugin.
 * @param plugin mdp_t requesting plugin unregistration.
 * @param renderer mdp_render_t struct containing renderer information.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_host_renderer_unregister(mdp_t *plugin, mdp_render_t *renderer)
{
	// TODO: Implement this function.
	return MDP_ERR_OK;
}


/**
 * mdp_host_window_register(): Register a window.
 * @param plugin mdp_t requesting window registration.
 * @param window Window to be registered.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_host_window_register(mdp_t *plugin, void *window)
{
	// Check the plugin window list to see if this window is already registered.
	for (list<mdpWindow_t>::iterator lstIter = PluginMgr::lstWindows.begin();
	     lstIter != PluginMgr::lstWindows.end(); lstIter++)
	{
		if ((*lstIter).window == window)
		{
			// Window is already registered.
			return -MDP_ERR_WINDOW_ALREADY_REGISTERED;
		}
	}
	
	// Window is not registered. Register it.
	mdpWindow_t win;
	win.window = window;
	win.owner = plugin;
	PluginMgr::lstWindows.push_back(win);
	
#ifdef _WIN32
	// Make sure the window has an icon.
	mdp_host_win32_check_icon(window);
#endif
	
	// Window is registered.
	return MDP_ERR_OK;
}


/**
 * mdp_host_window_unregister(): Unregister a window.
 * @param plugin mdp_t requesting window unregistration.
 * @param window Window to be unregistered.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_host_window_unregister(mdp_t *plugin, void *window)
{
	// Search for the window to be unregistered.
	for (list<mdpWindow_t>::iterator lstIter = PluginMgr::lstWindows.begin();
	     lstIter != PluginMgr::lstWindows.end(); lstIter++)
	{
		if ((*lstIter).window == window)
		{
			// Found the window.
			if ((*lstIter).owner == plugin)
			{
				// Owner is correct. Unregister the window.
				PluginMgr::lstWindows.erase(lstIter);
				return MDP_ERR_OK;
			}
			else
			{
				// Owner is incorrect. Return an error code.
				return -MDP_ERR_WINDOW_INVALID_WINDOW;
			}
		}
	}
	
	// Window not found. Return an error code.
	return -MDP_ERR_WINDOW_INVALID_WINDOW;
}
