/***************************************************************************
 * Gens: MDP Plugin Manager.                                               *
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

#include "pluginmgr.hpp"

// Message logging.
#include "macros/log_msg.h"

#include "emulator/g_main.hpp"
#include "util/file/rom.hpp"

// File management functions.
#include "util/file/file.hpp"

// libgsft includes.
#include "libgsft/gsft_file.h"

// CPU flags
#include "gens_core/misc/cpuflags.h"

// MDP Host Services
#include "mdp_host_gens.h"

// MDP error codes.
#include "mdp/mdp_error.h"

// opendir/closedir/readdir/etc
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>

// mingw doesn't provide S_ISLNK.
#ifndef S_ISLNK
#define S_ISLNK(x) (0)
#endif

// C includes.
#include <string.h>

// C++ includes
#include <string>
#include <utility>
#include <list>
using std::pair;
using std::string;
using std::list;

// Dynamic library loader.
#include "mdp/mdp_dlopen.h"

// Render Manager.
#include "rendermgr.hpp"


// Internal render plugins.
#include "render/normal/mdp_render_1x_plugin.h"
#include "render/double/mdp_render_2x_plugin.h"
static mdp_t* mdp_internal[] =
{
	&mdp_render_1x,
	&mdp_render_2x,
	NULL
};


/**
 * lstMDP: List containing all loaded plugins.
 * Incompat: MDP Incompatibility List.
 * lstMdpDLL: MDP DLL information.
 */
list<mdp_t*> PluginMgr::lstMDP;
MDP_Incompat PluginMgr::Incompat;
mapMdpDLL    PluginMgr::tblMdpDLL;


/**
 * List and map containing plugin menu items.
 */
list<mdpMenuItem_t> PluginMgr::lstMenuItems;
mapMenuItems PluginMgr::tblMenuItems;

/**
 * List containing plugin windows.
 */
list<mdpWindow_t> PluginMgr::lstWindows;

/**
 * List and map containing registered directories.
 */
list<mdpDir_t> PluginMgr::lstDirectories;
mapDirItems PluginMgr::tblDirectories;

/**
 * Map containing plugin configuration.
 */
mapPluginConfig PluginMgr::tblPluginConfig;


/**
 * init(): Initialize the plugin system.
 */
void PluginMgr::init(void)
{
	// Make sure all loaded plugins are shut down.
	end();
	
	// Load all internal plugins.
	unsigned int i = 0;
	while (mdp_internal[i])
	{
		loadPlugin(mdp_internal[i]);
		
		// Next plugin.
		i++;
	}
	
#if !defined(GENS_OS_WIN32)
	// Linux/UNIX: Load external plugins from the user's ~/.gens directory first.
	scanExternalPlugins(string(PathNames.Gens_Path) + GSFT_DIR_SEP_STR + "plugins");
#else	
	// Win32: Load external plugins from the Gens directory.
	scanExternalPlugins(string(PathNames.Gens_EXE_Path) + GSFT_DIR_SEP_STR + "plugins");
#endif /* GENS_OS_WIN32 */
	
#ifdef GENS_MDP_DIR
	// System-wide plugins.
	// If GENS_MDP_DIR is defined, load plugins from there.
	scanExternalPlugins(GENS_MDP_DIR);
#endif /* GENS_MDP_DIR */
}


/**
 * loadPlugin(): Attempt to load a plugin.
 * @param plugin Plugin struct.
 * @param filename Filename of the plugin. (Empty if internal.)
 * @return True if loaded; false if not.
 */
bool PluginMgr::loadPlugin(mdp_t *plugin, const string& filename)
{
	// Check the mdp_t version.
	if (MDP_VERSION_MAJOR(plugin->interfaceVersion) !=
	    MDP_VERSION_MAJOR(MDP_INTERFACE_VERSION))
	{
		// Incorrect major interface version.
		LOG_MSG(mdp, LOG_MSG_LEVEL_ERROR,
			"%s: MDP major interface version mismatch. (plugin == %d.%d.%d; host == %d.%d.%d)",
			File::GetNameFromPath(filename).c_str(),
			MDP_VERSION_MAJOR(plugin->interfaceVersion),
			MDP_VERSION_MINOR(plugin->interfaceVersion),
			MDP_VERSION_REVISION(plugin->interfaceVersion),
			MDP_VERSION_MAJOR(MDP_INTERFACE_VERSION),
			MDP_VERSION_MINOR(MDP_INTERFACE_VERSION),
			MDP_VERSION_REVISION(MDP_INTERFACE_VERSION));
		
		Incompat.add(plugin, -MDP_ERR_INCORRECT_MAJOR_VERSION, filename);
		return false;
	}
	
	// Check for a duplicated UUID.
	for (list<mdp_t*>::iterator iterMDP = lstMDP.begin();
	     iterMDP != lstMDP.end(); iterMDP++)
	{
		if (memcmp((*iterMDP)->uuid, plugin->uuid, sizeof(plugin->uuid)) == 0)
		{
			// Duplicated UUID.
			LOG_MSG(mdp, LOG_MSG_LEVEL_ERROR,
				"%s: Duplicated UUID. (uuid == %02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x)",
				File::GetNameFromPath(filename).c_str(),
				plugin->uuid[0], plugin->uuid[1], plugin->uuid[2], plugin->uuid[3],
				plugin->uuid[4], plugin->uuid[5],
				plugin->uuid[6], plugin->uuid[7],
				plugin->uuid[8], plugin->uuid[9],
				plugin->uuid[10], plugin->uuid[11], plugin->uuid[12], plugin->uuid[13], plugin->uuid[14], plugin->uuid[15]);
			
			Incompat.add(plugin, -MDP_ERR_DUPLICATE_UUID, filename);
			return false;
		}
	}
	
	// Check required CPU flags.
	uint32_t cpuFlagsRequired = plugin->cpuFlagsRequired;
	if ((cpuFlagsRequired & CPU_Flags) != cpuFlagsRequired)
	{
		// CPU does not support some required CPU flags.
		LOG_MSG(mdp, LOG_MSG_LEVEL_ERROR,
			"%s: Required CPU flags mismatch. (plugin requires 0x%08X; host has 0x%08X)",
			File::GetNameFromPath(filename).c_str(),
			plugin->cpuFlagsRequired,
			CPU_Flags);
		
		Incompat.add(plugin, -MDP_ERR_NEEDS_CPUFLAGS, filename);
		return false;
	}
	
	// TODO: Check the minor version.
	// Probably not needed right now, but may be needed later.
	
	// Run the plugin initialization function.
	if (plugin->func && plugin->func->init)
	{
		int rval = plugin->func->init(&Gens_MDP_Host);
		if (rval != MDP_ERR_OK)
		{
			// Error occurred while initializing the plugin.
			LOG_MSG(mdp, LOG_MSG_LEVEL_ERROR,
				"%s: mdp->func->init() failed: 0x%08X",
				File::GetNameFromPath(filename).c_str(), rval);
			
			Incompat.add(plugin, rval, filename);
			return false;
		}
	}
	
	// Add the plugin to the list of plugins.
	lstMDP.push_back(plugin);
	
	// Plugin loaded.
	return true;
}


struct pluginDirEntry
{
	string filename;
	bool isDirectory;
	
	bool operator< (const pluginDirEntry& compare) const
	{
#ifdef GENS_OS_WIN32
		return (strcasecmp(filename.c_str(), compare.filename.c_str()) < 0);
#else
		return (strcmp(filename.c_str(), compare.filename.c_str()) < 0);
#endif
	}
};


/**
 * scanExternalPlugins(): Scan for external plugins.
 * @param directory Directory to scan for plugins.
 * @param recursive If true, scans directories recursively.
 */
void PluginMgr::scanExternalPlugins(const string& directory, bool recursive)
{
	// Scan the plugin directory for external plugins.
	DIR *dir_mdp = opendir(directory.c_str());
	
	if (!dir_mdp)
	{
		// Could not open the MDP plugin directory.
		LOG_MSG(mdp, LOG_MSG_LEVEL_ERROR,
			"Could not open directory '%s': %s.",
			directory.c_str(), strerror(errno));
		return;
	}
	
	// Read all files in the directory.
	struct dirent *d_entry;
	size_t d_name_len;
	mode_t filetype;
	
	// Create a list of all directory entries first.
	list<pluginDirEntry> pluginEntries;
	pluginDirEntry curEntry;
	
	// glibc provides a d_type field in struct dirent, but some Linux
	// filesystems (e.g. xfs) don't properly set the value of d_type,
	// which leads to all files having the type DT_UNKNOWN.
	struct stat d_stat;
	
	while ((d_entry = readdir(dir_mdp)))
	{
		// Check the file type.
		curEntry.filename = string(directory) +
				    string(GSFT_DIR_SEP_STR) +
				    string(d_entry->d_name);
		stat(curEntry.filename.c_str(), &d_stat);
		filetype = d_stat.st_mode;
		
		if (S_ISDIR(filetype))
		{
			// Directory.
			if (recursive)
			{
				// Recursive scan is enabled.
				// Make sure the directory isn't "." or "..".
				if (strncmp(d_entry->d_name, ".", 2) &&
				    strncmp(d_entry->d_name, "..", 3))
				{
					// Directory is not "." or "..".
					
					// Scan the directory.
					curEntry.isDirectory = true;
					pluginEntries.push_back(curEntry);
				}
			}
		}
		else if (S_ISREG(filetype) || S_ISLNK(filetype))
		{
			// Regular file or symlink.
			
			// Check if the file extension matches the shared library extension.
			d_name_len = strlen(d_entry->d_name);
			if (d_name_len >= (sizeof(MDP_DLOPEN_EXT) - 1))
			{
				// Filename is long enough.
				
				// Compare the file extension.
				if (!strncasecmp(&d_entry->d_name[d_name_len - sizeof(MDP_DLOPEN_EXT) + 1],
						 MDP_DLOPEN_EXT, sizeof(MDP_DLOPEN_EXT) - 1))
				{	
					// File extension matches.
					// Found a plugin.
					curEntry.isDirectory = false;
					pluginEntries.push_back(curEntry);
				}
			}
		}
	}
	
	// Close the directory.
	closedir(dir_mdp);
	
	// Sort the list of plugins.
	pluginEntries.sort();
	
	// Load the plugins.
	for (list<pluginDirEntry>::iterator curPlugin = pluginEntries.begin();
	     curPlugin != pluginEntries.end(); curPlugin++)
	{
		if (((*curPlugin).isDirectory) && recursive)
		{
			// Scan the directory.
			scanExternalPlugins((*curPlugin).filename, recursive);
		}
		else
		{
			// Load the plugin.
			loadExternalPlugin((*curPlugin).filename);
		}
	}
}


/**
 * loadExternalPlugin(): Load an external plugin.
 * @param filename Filename of the external plugin.
 */
void PluginMgr::loadExternalPlugin(const string& filename)
{
	const char *err;
	
	void *dlhandle = mdp_dlopen(filename.c_str());
	
	if (!dlhandle)
	{
		Incompat.add(NULL, MDP_ERR_CANNOT_OPEN_DLL, filename);
		
		err = mdp_dlerror();
		LOG_MSG(mdp, LOG_MSG_LEVEL_ERROR,
			"Could not open external plugin '%s': %s",
			File::GetNameFromPath(filename).c_str(), err);
		
		mdp_dlerror_str_free(err);
		return;
	}
	
	// Attempt to load the mdp symbol.
	mdp_t *plugin = static_cast<mdp_t*>(mdp_dlsym(dlhandle, "mdp"));
	if (!plugin)
	{
		Incompat.add(NULL, MDP_ERR_NO_MDP_SYMBOL, filename);
		
		// NOTE: This will say "Unknown error." if the symbol was found,
		// but the symbol was defined as NULL. I'll fix this later.
		err = mdp_dlerror();
		LOG_MSG(mdp, LOG_MSG_LEVEL_ERROR,
			"\"mdp\" symbol not found in plugin '%s': %s",
			File::GetNameFromPath(filename).c_str(), err);
		
		mdp_dlerror_str_free(err);
		mdp_dlclose(dlhandle);
		return;
	}
	
	// Symbol loaded. Load the plugin.
	LOG_MSG(mdp, LOG_MSG_LEVEL_INFO,
		 "\"mdp\" symbol loaded from plugin: %s",
		 File::GetNameFromPath(filename).c_str());
	
	if (!loadPlugin(plugin, filename))
	{
		// Error loading the MDP symbol.
		// loadPlugin() already added the plugin to Incompat.
		// Unload the DLL.
		mdp_dlclose(dlhandle);
	}
	else
	{
		// Plugin loaded.
		mdpDLL_t dll;
		dll.dlhandle = dlhandle;
		dll.filename = filename;
		tblMdpDLL.insert(pairMdpDLL(plugin, dll));
	}
}


/**
 * end(): Shut down the plugin system.
 */
void PluginMgr::end(void)
{
	// Shut down all plugins.
	for (list<mdp_t*>::iterator curMDP = lstMDP.begin();
	     curMDP != lstMDP.end(); curMDP++)
	{
		mdp_func_t *func = (*curMDP)->func;
		if (func && func->end)
			func->end();
		
		// If this is an external plugin, unload the DLL.
		mapMdpDLL::iterator iter = tblMdpDLL.find((*curMDP));
		if (iter != tblMdpDLL.end())
		{
			mdpDLL_t &dll = (*iter).second;
			if (dll.dlhandle)
				mdp_dlclose(dll.dlhandle);
		}
	}
	
	// Clear all plugin lists and maps.
	lstMDP.clear();
	tblMdpDLL.clear();
	
	// Clear the Render Manager.
	RenderMgr::clear();
}
