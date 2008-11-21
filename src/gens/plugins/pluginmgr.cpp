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

#include "pluginmgr.hpp"

using std::vector;

// Render plugins
#include "render/normal/mdp_render_1x_plugin.h"
#include "render/double/mdp_render_2x_plugin.h"
#include "render/scanline/mdp_render_scanline_plugin.h"
#include "render/scanline_50/mdp_render_scanline_50_plugin.h"
#include "render/scanline_25/mdp_render_scanline_25_plugin.h"
#include "render/scale2x/mdp_render_scale2x_plugin.h"
#include "render/scale2x/mdp_render_scale3x_plugin.h"
#include "render/scale2x/mdp_render_scale4x_plugin.h"

// Internal plugins
static MDP_t* mdp_internal[] =
{
	&mdp_render_1x,
	&mdp_render_2x,
	&mdp_render_scanline,
	&mdp_render_scanline_50,
	&mdp_render_scanline_25,
	&mdp_render_scale2x,
	&mdp_render_scale3x,
	&mdp_render_scale4x,
	NULL
};


/**
 * vRenderPlugins: Vector containing render plugins.
 */
vector<MDP_t*> PluginMgr::vRenderPlugins;


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
		// TODO: Check plugin version information.
		switch (mdp_internal[i]->type)
		{
			case MDPT_RENDER:
				// Rendering plugin.
				vRenderPlugins.push_back(mdp_internal[i]);
				if (mdp_internal[i]->init)
					mdp_internal[i]->init();
				break;
			
			default:
				// Unknown plugin type.
				break;
		}
		
		// Next plugin.
		i++;
	}
}

/**
 * end(): Shut down the plugin system.
 */
void PluginMgr::end(void)
{
	// Shut down all render plugins.
	for (unsigned int i = 0; i < vRenderPlugins.size(); i++)
	{
		if (vRenderPlugins.at(i)->end != NULL)
			vRenderPlugins.at(i)->end();
	}
	
	// Clear the vector of render plugins.
	vRenderPlugins.clear();
}
