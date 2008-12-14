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

// C++ includes.
#include <algorithm>
#include <string>
#include <list>
using std::string;
using std::list;

#include "mdp_host_gens_cpp.hpp"

// MDP includes.
#include "mdp/mdp_error.h"

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
	return MDP_ERR_OK;
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
	return MDP_ERR_OK;
}
