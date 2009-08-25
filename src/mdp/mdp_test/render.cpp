/***************************************************************************
 * MDP: Mega Drive Plugins - Test Suite.                                   *
 * render.cpp: Renderer Tests.                                             *
 *                                                                         *
 * Copyright (c) 2008-2009 by David Korth.                                 *
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

#include "render.hpp"
#include "main.h"

#include "mdp/mdp_error.h"

// C includes.
#include <string.h>
#include <stdint.h>

// C++ includes.
#include <list>
using std::list;

// List of plugins.
list<mdp_render_t*> lstPlugins;
typedef list<mdp_render_t*>::iterator iterPlugin_t;


/**
 * mdp_host_renderer_register(): Register a renderer plugin.
 * @param plugin Plugin requesting registration.
 * @param renderer Renderer plugin.
 * @return MDP error code.
 */
int mdp_host_renderer_register(mdp_t *plugin, mdp_render_t *renderer)
{
	if (!plugin)
	{
		TEST_WARN_ARGS("host_srv->%s() called with NULL plugin.", __func__);
		return -MDP_ERR_INVALID_PARAMETERS;
	}
	if (!renderer)
	{
		TEST_WARN_ARGS("host_srv->%s() called with NULL renderer.", __func__);
		return -MDP_ERR_INVALID_PARAMETERS;
	}
	if (!renderer->tag)
	{
		TEST_WARN_ARGS("host_srv->%s() called with invalid render information. (renderer->tag == NULL)", __func__);
		return -MDP_ERR_RENDER_INVALID_RENDERINFO;
	}
	
	// Check if a plugin with this tag already exists.
	for (iterPlugin_t iter = lstPlugins.begin();
	     iter != lstPlugins.end(); iter++)
	{
		if (!strcmp((*iter)->tag, renderer->tag))
		{
			// Tag already exists.
			TEST_WARN_ARGS("host_srv->%s(): Renderer with tag '%s' already exists.",
				       __func__, renderer->tag);
			return -MDP_ERR_RENDER_DUPLICATE_TAG;
		}
	}
	
	// Add the renderer to the list.
	// TODO: Save the owner of the renderer.
	lstPlugins.push_back(renderer);
	TEST_INFO_ARGS("host_srv->%s(): Registered renderer '%s'.",
		       __func__, renderer->tag);
	return MDP_ERR_OK;
}


/**
 * mdp_host_renderer_unregister(): Unregister a renderer plugin.
 * @param plugin Plugin requesting registration.
 * @param renderer Renderer plugin.
 * @return MDP error code.
 */
int mdp_host_renderer_unregister(mdp_t *plugin, mdp_render_t *renderer)
{
	if (!plugin)
	{
		TEST_WARN_ARGS("host_srv->%s() called with NULL plugin.", __func__);
		return -MDP_ERR_INVALID_PARAMETERS;
	}
	if (!renderer)
	{
		TEST_WARN_ARGS("host_srv->%s() called with NULL renderer.", __func__);
		return -MDP_ERR_INVALID_PARAMETERS;
	}
	
	// Check if this renderer is registered.
	for (iterPlugin_t iter = lstPlugins.begin();
	     iter != lstPlugins.end(); iter++)
	{
		if ((*iter) == renderer)
		{
			// Found the renderer.
			// TODO: Check the ownership of the renderer.
			lstPlugins.erase(iter);
			TEST_INFO_ARGS("host_srv->%s(): Unregistered renderer '%s'.",
				       __func__, renderer->tag);
			return MDP_ERR_OK;
		}
	}
	
	// Renderer not found.
	if (renderer->tag)
	{
		TEST_WARN_ARGS("host_srv->%s(): Renderer '%s' not registered.",
			       __func__, renderer->tag);
	}
	else
	{
		// TODO: Make this 64-bit safe.
		TEST_WARN_ARGS("host_srv->%s(): Renderer '0x%08X' not registered.",
			       __func__, (intptr_t)renderer);
	}
	
	// TODO: Dedicated error code for this.
	return -MDP_ERR_INVALID_PARAMETERS;
}
