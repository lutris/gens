/***************************************************************************
 * Gens: MDP Render Plugin Manager.                                        *
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

#ifndef GENS_RENDERMGR_HPP
#define GENS_RENDERMGR_HPP

#ifdef __cplusplus

// MDP includes.
#include "mdp/mdp.h"
#include "mdp/mdp_render.h"

#include "libgsft/gsft_hashtable.hpp"

// C++ includes.
#include <string>
#include <list>

// Render plugins.
typedef GSFT_HASHTABLE<std::string, std::list<mdp_render_t*>::iterator> mapRenderPlugin;
typedef std::pair<std::string, std::list<mdp_render_t*>::iterator> pairRenderPlugin;

class RenderMgr
{
	public:
		static void clear();
		static bool tagExists(std::string tag);
		
		static int addRenderPlugin(mdp_render_t *renderer);
		
		static bool empty(void) { return lstRenderPlugins.empty(); }
		static int size(void) { return lstRenderPlugins.size(); }
		
		// List iterator functions.
		static std::list<mdp_render_t*>::iterator begin(void)
		{
			return lstRenderPlugins.begin();
		}
		static std::list<mdp_render_t*>::iterator end(void)
		{
			return lstRenderPlugins.end();
		}
		
		static std::list<mdp_render_t*>::iterator getIterFromTag(std::string tag);
		static std::list<mdp_render_t*>::iterator getIterFromIndex(unsigned int index);
		
	private:
		// List and map containing all loaded render plugins.
		static std::list<mdp_render_t*> lstRenderPlugins;
		static mapRenderPlugin tblRenderPlugins;
};

#endif

#endif /* GENS_RENDERMGR_HPP */
