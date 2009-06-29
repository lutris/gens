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

#include "rendermgr.hpp"

// MDP error codes.
#include "mdp/mdp_error.h"

// C++ includes.
#include <algorithm>
#include <string>
#include <list>
using std::string;
using std::list;


/**
 * List and map containing all loaded render plugins.
 */
list<mdp_render_t*>	RenderMgr::lstRenderPlugins;
mapRenderPlugin		RenderMgr::tblRenderPlugins;


/**
 * clear(): Clear the plugin list and map.
 */
void RenderMgr::clear(void)
{
	lstRenderPlugins.clear();
	tblRenderPlugins.clear();
}


bool RenderMgr::tagExists(string tag)
{
	std::transform(tag.begin(), tag.end(), tag.begin(), ::tolower);
	mapRenderPlugin::iterator renderMDP = tblRenderPlugins.find(tag);
	
	return (renderMDP != tblRenderPlugins.end());
}


/**
 * addRenderPlugin(): Add a render plugin.
 * @param renderer Render plugin.
 */
int RenderMgr::addRenderPlugin(mdp_render_t *renderer)
{
	if (!renderer)
		return -MDP_ERR_INVALID_PARAMETERS;
	if (!renderer->tag)
		return -MDP_ERR_RENDER_INVALID_RENDERINFO;
	
	// Check if a plugin with this tag already exists.
	if (tagExists(renderer->tag))
	{
		// Plugin with this tag already exists.
		// TODO: Show an error.
		return -MDP_ERR_RENDER_DUPLICATE_TAG;
	}
	
	// Add the renderer to the list.
	lstRenderPlugins.push_back(renderer);
	
	// Add the renderer tag to the map.
	string tag = renderer->tag;
	std::transform(tag.begin(), tag.end(), tag.begin(), ::tolower);
	
	list<mdp_render_t*>::iterator lstIter = lstRenderPlugins.end();
	lstIter--;
	tblRenderPlugins.insert(pairRenderPlugin(tag, lstIter));
	
	// Renderer added.
	return MDP_ERR_OK;
}


/**
 * getIterFromTag(): Get a render plugin iterator from its tag.
 * @param tag Plugin tag.
 * @return Render plugin iterator from lstRenderPlugins.
 */
list<mdp_render_t*>::iterator RenderMgr::getIterFromTag(string tag)
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


/**
 * getIterFromIndex(): Get a render plugin iterator from its index.
 * @param index Plugin index.
 * @return Render plugin iterator from lstRenderPlugins.
 */
list<mdp_render_t*>::iterator RenderMgr::getIterFromIndex(unsigned int index)
{
	list<mdp_render_t*>::iterator renderIter = lstRenderPlugins.begin();
	
	for (unsigned int i = 1; i < index; i++)
	{
		renderIter++;
		if (renderIter == lstRenderPlugins.end())
			break;
	}
	
	return renderIter;
}
