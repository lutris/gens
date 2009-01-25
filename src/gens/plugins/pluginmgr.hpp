/***************************************************************************
 * Gens: Plugin Manager.                                                   *
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

#ifndef GENS_PLUGINMGR_HPP
#define GENS_PLUGINMGR_HPP

#include "mdp/mdp.h"
#include "mdp/mdp_render.h"
#include "mdp/mdp_host.h"

#include <string>
#include <list>
#include "macros/hashtable.hpp"

typedef GENS_HASHTABLE<std::string, std::list<MDP_Render_t*>::iterator> mapRenderPlugin;
typedef std::pair<std::string, std::list<MDP_Render_t*>::iterator> pairRenderPlugin;

// Menu items
struct mdpMenuItem_t
{
	uint16_t 		id;
	mdp_menu_handler_fn	handler;
	MDP_t			*owner;
	
	// Attributes
	std::string		text;
	bool			checked;
};

typedef GENS_HASHTABLE<uint16_t, std::list<mdpMenuItem_t>::iterator> mapMenuItems;
typedef std::pair<uint16_t, std::list<mdpMenuItem_t>::iterator> pairMenuItems;

// Windows
struct mdpWindow_t
{
	void	*window;
	MDP_t	*owner;
};

class PluginMgr
{
	public:
		static void init(void);
		static void end(void);
		
		static std::list<MDP_Render_t*>::iterator getMDPIterFromTag_Render(std::string tag);
		
		// List containing all loaded plugins.
		static std::list<MDP_t*> lstMDP;
		
		// List and map containing all loaded render plugins.
		static std::list<MDP_Render_t*> lstRenderPlugins;
		static mapRenderPlugin tblRenderPlugins;
		
		// List and map containing plugin menu items.
		static std::list<mdpMenuItem_t> lstMenuItems;
		static mapMenuItems tblMenuItems;
		
		// List containing plugin windows.
		static std::list<mdpWindow_t> lstWindows;
		
	protected:
		static bool loadPlugin(MDP_t *plugin);
		static void scanExternalPlugins(const std::string& directory, bool recursive = true);
		static void loadExternalPlugin(const std::string& filename);
		
		// TODO: Replace with MDP_Host_t->registerRenderer().
		//static bool initPlugin_Render(MDP_t *plugin);
};

#endif /* GENS_PLUGINMGR_HPP */
