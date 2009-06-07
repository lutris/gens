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

#ifndef GENS_PLUGINMGR_HPP
#define GENS_PLUGINMGR_HPP

#include "mdp/mdp.h"
#include "mdp/mdp_render.h"
#include "mdp/mdp_host.h"

// MDP Incompatibility List.
#include "mdp_incompat.hpp"

#ifdef __cplusplus

// Hashtable wrapper.
#include "libgsft/gsft_hashtable.hpp"

// C++ includes.
#include <string>
#include <list>

// Plugin DLL information.
typedef struct _mdpDLL_t
{
	void		*dlhandle;
	std::string	filename;
} mdpDLL_t;
typedef GSFT_HASHTABLE<mdp_t*, mdpDLL_t> mapMdpDLL;
typedef std::pair<mdp_t*, mdpDLL_t> pairMdpDLL;

// Menu items
typedef struct _mdpMenuItem_t
{
	uint16_t 		id;
	mdp_menu_handler_fn	handler;
	mdp_t			*owner;
	
	// Attributes
	std::string		text;
	bool			checked;
} mdpMenuItem_t;

typedef GSFT_HASHTABLE<uint16_t, std::list<mdpMenuItem_t>::iterator> mapMenuItems;
typedef std::pair<uint16_t, std::list<mdpMenuItem_t>::iterator> pairMenuItems;

// Windows.
typedef struct _mdpWindow_t
{
	void	*window;
	mdp_t	*owner;
} mdpWindow_t;

// Directories.
typedef struct _mdpDir_t
{
	int		id;
	std::string	name;
	
	mdp_dir_get_fn	get;
	mdp_dir_set_fn	set;
	
	mdp_t		*owner;
} mdpDir_t;

typedef GSFT_HASHTABLE<int, std::list<mdpDir_t>::iterator> mapDirItems;
typedef std::pair<int, std::list<mdpDir_t>::iterator> pairDirItems;

// Plugin configuration.
typedef GSFT_HASHTABLE<std::string, std::string> mapConfigItems;
typedef std::pair<std::string, std::string> pairConfigItems;

typedef GSFT_HASHTABLE<mdp_t*, mapConfigItems> mapPluginConfig;
typedef std::pair<mdp_t*, mapConfigItems> pairPluginConfig;

class PluginMgr
{
	public:
		static void init(void);
		static void end(void);
		
		static std::list<mdp_render_t*>::iterator getMDPIterFromTag_Render(std::string tag);
		
		/**
		 * lstMDP: List containing all loaded plugins.
		 * Incompat: MDP Incompatibility List.
		 * lstMdpDLL: MDP DLL information.
		 */
		static std::list<mdp_t*> lstMDP;
		static MDP_Incompat Incompat;
		static mapMdpDLL tblMdpDLL;
		
		// List and map containing plugin menu items.
		static std::list<mdpMenuItem_t> lstMenuItems;
		static mapMenuItems tblMenuItems;
		
		// List containing plugin windows.
		static std::list<mdpWindow_t> lstWindows;
		
		// List and map containing registered directories.
		static std::list<mdpDir_t> lstDirectories;
		static mapDirItems tblDirectories;
		
		// Map containing plugin configuration.
		static mapPluginConfig tblPluginConfig;
	
	protected:
		static bool loadPlugin(mdp_t *plugin, const std::string& filename = "");
		static void scanExternalPlugins(const std::string& directory, bool recursive = true);
		static void loadExternalPlugin(const std::string& filename);
		
		static void errLoadPlugin(mdp_t *plugin, int err, const std::string& filename = "");
};

#endif /* __cplusplus */

#endif /* GENS_PLUGINMGR_HPP */
