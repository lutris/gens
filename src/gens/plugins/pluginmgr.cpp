/***************************************************************************
 * Gens: Plugin Manager.                                                   *
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
#endif /* HAVE_CONFIG_H */

// Message logging.
#include "macros/log_msg.h"

#include "pluginmgr.hpp"
#include "macros/hashtable.hpp"

#include "emulator/g_main.hpp"
#include "util/file/rom.hpp"

// File management functions.
#include "util/file/file.hpp"

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
#include <cstring>

// mingw doesn't provide S_ISLNK.
#ifndef S_ISLNK
#define S_ISLNK(x) (0)
#endif

// C++ includes
#include <algorithm>
#include <string>
#include <utility>
#include <list>
using std::pair;
using std::string;
using std::list;

// Libtool Dynamic Loader
#include <ltdl.h>

// Internal render plugins.
#include "render/normal/mdp_render_1x_plugin.h"
#include "render/double/mdp_render_2x_plugin.h"
static MDP_t* mdp_internal[] =
{
	&mdp_render_1x,
	&mdp_render_2x,
	NULL
};


/**
 * lstMDP: List containing all loaded plugins.
 * Incompat: MDP Incompatibility List.
 */
list<MDP_t*> PluginMgr::lstMDP;
MDP_Incompat PluginMgr::Incompat;


/**
 * lstRenderPlugins, tblRenderPlugins: List and map containing all loaded render plugins.
 */
list<MDP_Render_t*> PluginMgr::lstRenderPlugins;
mapRenderPlugin PluginMgr::tblRenderPlugins;

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
	
	// Load all external plugins.
	#ifdef GENS_OS_WIN32
		scanExternalPlugins(PathNames.Gens_EXE_Path);
		
		// Wine path
		scanExternalPlugins("Z:\\home\\david\\programming\\gens\\debug-win32\\src\\mdp");
		
		// VMware path
		scanExternalPlugins("Z:\\david\\programming\\gens\\debug-win32\\src\\mdp");
	#endif /* GENS_OS_WIN32 */
	
	#ifdef GENS_MDP_DIR
		scanExternalPlugins(GENS_MDP_DIR);
		scanExternalPlugins("/home/david/programming/gens/debug-linux/src/mdp");
	#endif /* GENS_MDP_DIR */
}


/**
 * loadPlugin(): Attempt to load a plugin.
 * @param plugin Plugin struct.
 * @param filename Filename of the plugin. (Empty if internal.)
 * @return True if loaded; false if not.
 */
bool PluginMgr::loadPlugin(MDP_t *plugin, const string& filename)
{
	// Check the MDP_t version.
	if (MDP_VERSION_MAJOR(plugin->interfaceVersion) !=
	    MDP_VERSION_MAJOR(MDP_INTERFACE_VERSION))
	{
		// Incorrect major interface version.
		Incompat.add(plugin, -MDP_ERR_INCORRECT_MAJOR_VERSION, filename);
		return false;
	}
	
	// Check the license.
	if (!plugin->desc || !plugin->desc->license)
	{
		// No license.
		Incompat.add(plugin, -MDP_ERR_INVALID_LICENSE, filename);
		return false;
	}
	
	if (strncasecmp(plugin->desc->license, MDP_LICENSE_GPL_2,	sizeof(MDP_LICENSE_GPL_2))	&&
	    strncasecmp(plugin->desc->license, MDP_LICENSE_GPL_3,	sizeof(MDP_LICENSE_GPL_3))	&&
	    strncasecmp(plugin->desc->license, MDP_LICENSE_LGPL_2,	sizeof(MDP_LICENSE_LGPL_2))	&&
	    strncasecmp(plugin->desc->license, MDP_LICENSE_LGPL_21,	sizeof(MDP_LICENSE_LGPL_21))	&&
	    strncasecmp(plugin->desc->license, MDP_LICENSE_LGPL_3,	sizeof(MDP_LICENSE_LGPL_3))	&&
	    strncasecmp(plugin->desc->license, MDP_LICENSE_BSD,		sizeof(MDP_LICENSE_BSD))	&&
	    strncasecmp(plugin->desc->license, MDP_LICENSE_PD,		sizeof(MDP_LICENSE_PD)))
	{
		// Invalid license.
		Incompat.add(plugin, -MDP_ERR_INVALID_LICENSE, filename);
		return false;
	}
	
	// Check required CPU flags.
	uint32_t cpuFlagsRequired = plugin->cpuFlagsRequired;
	if ((cpuFlagsRequired & CPU_Flags) != cpuFlagsRequired)
	{
		// CPU does not support some required CPU flags.
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
			"Could not open MDP plugin directory: %s",
			directory.c_str());
		return;
	}
	
	// Read all files in the directory.
	struct dirent *d_entry;
	size_t d_name_len;
	mode_t filetype;
	
	// Create a list of all directory entries first.
	list<pluginDirEntry> pluginEntries;
	pluginDirEntry curEntry;
	
	// Required if libc doesn't provide the dirent->d_type field.
	#ifndef _DIRENT_HAVE_D_TYPE
		struct stat dirstat;
	#endif /* !_DIRENT_HAVE_D_TYPE */
	
	while ((d_entry = readdir(dir_mdp)))
	{
		// Check the file type.
		#ifdef _DIRENT_HAVE_D_TYPE
			// libc provides the dirent->d_type field.
			filetype = DTTOIF(d_entry->d_type);
		#else /* !_DIRENT_HAVE_D_TYPE */
			// libc does not provide the dirent->d_type field.
			// mingw unfortunately doesn't. :(
			curEntry.filename = directory + GENS_DIR_SEPARATOR_STR + d_entry->d_name;
			stat(curEntry.filename.c_str(), &dirstat);
			filetype = dirstat.st_mode;
		#endif /* _DIRENT_HAVE_D_TYPE */
		
		if (S_ISDIR(filetype))
		{
			// Directory.
			if (recursive)
			{
				// Recursive scan is enabled.
				// Make sure the directory isn't "." or "..".
				if (memcmp(d_entry->d_name, ".", 2) &&
				    memcmp(d_entry->d_name, "..", 3))
				{
					// Directory is not "." or "..".
					
					// Scan the directory.
					#ifdef _DIRENT_HAVE_D_TYPE
						curEntry.filename = directory + GENS_DIR_SEPARATOR_STR + d_entry->d_name;
					#endif /* _DIRENT_HAVE_D_TYPE */
					curEntry.isDirectory = true;
					pluginEntries.push_back(curEntry);
				}
			}
		}
		else if (S_ISREG(filetype) || S_ISLNK(filetype))
		{
			// Regular file or symlink.
			
			// Check if the file extension matches libltdl's shared library extension.
			d_name_len = strlen(d_entry->d_name);
			if (d_name_len >= (sizeof(LTDL_SHLIB_EXT) - 1))
			{
				// Filename is long enough.
				
				// Compare the file extension.
				if (!strncasecmp(&d_entry->d_name[d_name_len - sizeof(LTDL_SHLIB_EXT) + 1],
						 LTDL_SHLIB_EXT, sizeof(LTDL_SHLIB_EXT) - 1))
				{	
					// File extension matches.
					// Found a plugin.
					#ifdef _DIRENT_HAVE_D_TYPE
						curEntry.filename = directory + GENS_DIR_SEPARATOR_STR + d_entry->d_name;
					#endif
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
	lt_dlinit();
	lt_dlhandle handle = lt_dlopen(filename.c_str());
	
	if (!handle)
	{
		Incompat.add(NULL, MDP_ERR_CANNOT_OPEN_DLL, filename);
		LOG_MSG(mdp, LOG_MSG_LEVEL_ERROR,
			"Could not open external plugin: %s",
			File::GetNameFromPath(filename).c_str());
		return;
	}
	
	// Attempt to load the mdp symbol.
	MDP_t *plugin = static_cast<MDP_t*>(lt_dlsym(handle, "mdp"));
	if (!plugin)
	{
		Incompat.add(NULL, MDP_ERR_NO_MDP_SYMBOL, filename);
		LOG_MSG(mdp, LOG_MSG_LEVEL_ERROR,
			"\"mdp\" symbol not found in plugin: %s",
			File::GetNameFromPath(filename).c_str());
		lt_dlclose(handle);
		return;
	}
	
	// Symbol loaded. Load the plugin.
	LOG_MSG(mdp, LOG_MSG_LEVEL_INFO,
		 "\"mdp\" symbol loaded from plugin: %s",
		 File::GetNameFromPath(filename).c_str());
	loadPlugin(plugin, filename);
}


/**
 * end(): Shut down the plugin system.
 */
void PluginMgr::end(void)
{
	// Shut down all render plugins.
	for (list<MDP_t*>::iterator curMDP = lstMDP.begin();
	     curMDP != lstMDP.end(); curMDP++)
	{
		MDP_Func_t *func = (*curMDP)->func;
		if (func && func->end)
			func->end();
	}
	
	// Clear all plugin lists and maps.
	lstMDP.clear();
	
	lstRenderPlugins.clear();
	tblRenderPlugins.clear();
}


/**
 * getMDPIterFromTag_Render(): Get a render plugin iterator from its tag.
 * @param tag Plugin tag.
 * @return Render plugin iterator from lstRenderPlugins.
 */
std::list<MDP_Render_t*>::iterator PluginMgr::getMDPIterFromTag_Render(string tag)
{
	if (tag.empty())
		return lstRenderPlugins.end();
	
	// Search for the plugin tag.
	std::transform(tag.begin(), tag.end(), tag.begin(), ::tolower);
	mapRenderPlugin::iterator renderMDP = tblRenderPlugins.find(tag);
	if (renderMDP == tblRenderPlugins.end())
		return lstRenderPlugins.end();
	else
		return (*renderMDP).second;
}
