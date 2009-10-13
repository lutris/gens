/***************************************************************************
 * Gens: MDP: Mega Drive Plugin - Host Services. (Directory Registration)  *
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

#include "mdp_host_gens_dir.hpp"
#include "mdp/mdp_error.h"

// Main emulation functions.
#include "emulator/g_main.hpp"

#include "pluginmgr.hpp"

// libgsft includes.
#include "libgsft/gsft_strlcpy.h"

// C includes.
#include <string.h>

// C++ includes.
#include <string>
#include <list>
using std::string;
using std::list;


/**
 * mdp_host_dir_get_default_save_path(): Get the default save path.
 * @param buf Buffer to store the default save path in.
 * @param size Size of the buffer.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_host_dir_get_default_save_path(char *buf, unsigned int size)
{
	if (!buf || !size)
	{
		// Invalid parameters.
		return -MDP_ERR_INVALID_PARAMETERS;
	}
	
	// TODO: Return an error if the buffer is too small.
#ifdef GENS_OS_WIN32
	// Win32's default save path is ".\\".
	// Return the full save path instead.
	strlcpy(buf, PathNames.Gens_EXE_Path, size);
#else
	// Get the actual default save path.
	get_default_save_path(buf, size);
#endif
	
	return MDP_ERR_OK;
}


/**
 * mdp_host_dir_register(): Register a directory with the "Directory Configuration" window.
 * @param plugin Plugin requesting directory registration.
 * @param dir_name Directory name. (aka what the directory is)
 * @param get_fn Directory "get" function.
 * @param set_fn Directory "set" function.
 * @return >=0 == directory ID; <0 == MDP error code.
 */
int MDP_FNCALL mdp_host_dir_register(mdp_t *plugin, const char *dir_name,
				     mdp_dir_get_fn get_fn, mdp_dir_set_fn set_fn)
{
	if (!plugin || !dir_name || !get_fn || !set_fn)
	{
		// Invalid parameters.
		return -MDP_ERR_INVALID_PARAMETERS;
	}
	
	// TODO: Enforce unique directory names?
	
	// Locate the highest directory ID.
	int dir_id = 0;
	
	for (list<mdpDir_t>::iterator iter = PluginMgr::lstDirectories.begin();
	     iter != PluginMgr::lstDirectories.end(); iter++)
	{
		if ((*iter).id >= dir_id)
			dir_id = (*iter).id + 1;
	}
	
	if (dir_id < 0)
	{
		// Too many directories.
		return -MDP_ERR_OUT_OF_MEMORY;
	}
	
	// Add the directory to the list.
	mdpDir_t dir;
	dir.id = dir_id;
	dir.name = string(dir_name);
	dir.get = get_fn;
	dir.set = set_fn;
	dir.owner = plugin;
	PluginMgr::lstDirectories.push_back(dir);
	
	// Add the directory ID to the map.
	list<mdpDir_t>::iterator lstIter = PluginMgr::lstDirectories.end();
	lstIter--;
	PluginMgr::tblDirectories.insert(pairDirItems(dir_id, lstIter));
	
	// Return the directory ID.
	return dir_id;
}


/**
 * mdp_host_dir_register(): Unregister a directory.
 * @param plugin Plugin requesting directory unregistration.
 * @param dir_id Directory ID.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_host_dir_unregister(mdp_t *plugin, int dir_id)
{
	if (!plugin)
		return -MDP_ERR_INVALID_PARAMETERS;
	if (dir_id < 0)
		return -MDP_ERR_DIR_INVALID_DIRID;
	
	// Search for the directory.
	mapDirItems::iterator curDirItem = PluginMgr::tblDirectories.find(dir_id);
	if (curDirItem == PluginMgr::tblDirectories.end())
	{
		// Directory not found.
		return -MDP_ERR_DIR_INVALID_DIRID;
	}
	
	// Get the list iterator.
	list<mdpDir_t>::iterator lstIter = (*curDirItem).second;
	
	// Check if the directory is owned by this plugin.
	if ((*lstIter).owner != plugin)
	{
		// Not owned by the plugin.
		return -MDP_ERR_DIR_INVALID_DIRID;
	}
	
	// Menu item is owned by the plugin. Delete it.
	PluginMgr::tblDirectories.erase(dir_id);
	PluginMgr::lstDirectories.erase(lstIter);
	
	return MDP_ERR_OK;
}
