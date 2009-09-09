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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// C++ includes.
#include <list>
using std::list;

// List of plugins.
list<mdp_render_t*> lstPlugins;
typedef list<mdp_render_t*>::iterator iterPlugin_t;

static int renderer_test(mdp_render_t *renderer);


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
	if (renderer->scale < 0)
	{
		TEST_WARN_ARGS("host_srv->%s() called with invalid render information. (renderer->scale == %d)", __func__, renderer->scale);
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
		TEST_WARN_ARGS("host_srv->%s(): Renderer 0x%08X not registered.",
			       __func__, (intptr_t)renderer);
	}
	
	// TODO: Dedicated error code for this.
	return -MDP_ERR_INVALID_PARAMETERS;
}


/**
 * renderer_clear(): Clear the renderer plugin list.
 * @return 0 if no plugins were in the list; non-zero if plugins were in the list.
 */
int renderer_clear(void)
{
	if (lstPlugins.empty())
		return 0;
	
	// Plugin list isn't empty.
	const int plsize = lstPlugins.size();
	
	for (iterPlugin_t iter = lstPlugins.begin();
	     iter != lstPlugins.end(); iter++)
	{
		const char *tag = (*iter)->tag;
		if (tag)
		{
			TEST_FAIL_ARGS("Renderer '%s' was not unregistered at shutdown.", tag);
		}
		else
		{
			// TODO: Make this 64-bit safe.
			TEST_FAIL_ARGS("Renderer 0x%08X was not unregistered at shutdown.", (intptr_t)(*iter));
		}
	}
	
	lstPlugins.clear();
	return plsize;
}


/**
 * renderer_test_all(): Test all renderers.
 * @return 0 if all renderers passed; number of renderers that failed otherwise.
 */
int renderer_test_all(void)
{
	int num_failed = 0;
	
	for (iterPlugin_t iter = lstPlugins.begin();
	     iter != lstPlugins.end(); iter++)
	{
		int rval = renderer_test(*iter);
		if (rval != 0)
		{
			// Renderer failed.
			num_failed++;
		}
	}
	
	return num_failed;
}


/**
 * renderer_test(): Test a renderer.
 * @param renderer Renderer.
 */
static int renderer_test(mdp_render_t *renderer)
{
	if (!renderer)
		return -MDP_ERR_INVALID_PARAMETERS;
	
	int passed = 1;
	
	putchar('\n');
	TEST_START_ARGS("Testing renderer '%s'", renderer->tag);
	putchar('\n');
	
	// Print supported video formats.
	TEST_INFO_ARGS("renderer_test(): Supported video formats for renderer '%s':", renderer->tag);
	if (renderer->flags & MDP_RENDER_FLAG_RGB_555to555)
		TEST_INFO_ARGS("\tRGB555 -> RGB555");
	if (renderer->flags & MDP_RENDER_FLAG_RGB_555to565)
		TEST_INFO_ARGS("\tRGB555 -> RGB565");
	if (renderer->flags & MDP_RENDER_FLAG_RGB_555to888)
		TEST_INFO_ARGS("\tRGB555 -> RGB888");
	if (renderer->flags & MDP_RENDER_FLAG_RGB_565to555)
		TEST_INFO_ARGS("\tRGB565 -> RGB555");
	if (renderer->flags & MDP_RENDER_FLAG_RGB_565to565)
		TEST_INFO_ARGS("\tRGB565 -> RGB565");
	if (renderer->flags & MDP_RENDER_FLAG_RGB_565to888)
		TEST_INFO_ARGS("\tRGB565 -> RGB888");
	if (renderer->flags & MDP_RENDER_FLAG_RGB_888to555)
		TEST_INFO_ARGS("\tRGB888 -> RGB555");
	if (renderer->flags & MDP_RENDER_FLAG_RGB_888to565)
		TEST_INFO_ARGS("\tRGB888 -> RGB565");
	if (renderer->flags & MDP_RENDER_FLAG_RGB_888to888)
		TEST_INFO_ARGS("\tRGB888 -> RGB888");
	
	// Allocate a source buffer large enough for 336x240, 32-bit color.
	// The buffer also has 16 bytes of padding at the beginning.
	int src_size = (336*240*4)+16;
	uint8_t *src = (uint8_t*)malloc(src_size);
	
	// Allocate a destination buffer large enough for 3 times 336*240.
	int dest_size = (336 * renderer->scale) * (240 * renderer->scale) * 4;
	uint8_t *dest = (uint8_t*)malloc(dest_size * 3);
	
	// Test 565to565.
	TEST_START("Testing RGB565 -> RGB565");
	putchar('\n');
	
	// TODO: More video modes.
	
	// Fill the source and destination buffers with a random value.
	uint8_t rnd_chr = 0x7A; // TODO: Use a random value.
	memset(src, rnd_chr, src_size);
	memset(dest, rnd_chr, dest_size * 3);
	
	// Create the render information.
	mdp_render_info_t render_info;
	render_info.destScreen = &dest[dest_size];		// Start in the middle of the dest buffer.
	render_info.mdScreen = &src[16];			// Start at 16 bytes into the src buffer.
	render_info.destPitch = (320 * renderer->scale) * 2;	// Destination pitch.
	render_info.srcPitch = 336 * 2;				// Source pitch.
	render_info.width = 320;				// Width.
	render_info.height = 224;				// Height.
	render_info.cpuFlags = 0xFFFFFFFF;			// TODO: Use actual CPU flags.
	render_info.vmodeFlags = MDP_RENDER_VMODE_CREATE(MDP_RENDER_VMODE_RGB_565, MDP_RENDER_VMODE_RGB_565);
	render_info.data = NULL;
	
	// Run the renderer.
	renderer->blit(&render_info);
	
	// Check the source buffer to make sure it wasn't changed.
	uint8_t *src_chk = &src[src_size-1];
	unsigned int i;
	for (i = src_size; i != 0; i--)
	{
		if (*src_chk != rnd_chr)
		{
			// Source buffer was changed!
			TEST_FAIL("Source buffer was modified!");
			passed = 0;
			break;
		}
		src_chk--;
	}
	if (i == 0)
		TEST_PASS_MSG("Source buffer was not modified.");
	
	// Check the destination buffer to make sure the correct areas were written.
	
	// Check the prebuffer.
	uint8_t *dest_chk = &dest[dest_size-1];
	for (i = dest_size; i != 0; i--)
	{
		if (*dest_chk != rnd_chr)
		{
			// Destination prebuffer was changed!
			TEST_FAIL("Destination buffer was modified before the visible area.");
			passed = 0;
			break;
		}
		dest_chk--;
	}
	if (i == 0)
		TEST_PASS_MSG("Destination buffer was not modified before the visible area.");
	
	// TODO: Check the main buffer.
	TEST_INFO("TODO: Check the main rendering buffer.");
	
	// Check the postbuffer.
	dest_chk = &dest[(dest_size*3)-1];
	for (i = dest_size; i != 0; i--)
	{
		if (*dest_chk != rnd_chr)
		{
			// Destination postbuffer was changed!
			TEST_FAIL("Destination buffer was modified after the visible area.");
			passed = 0;
			break;
		}
		dest_chk--;
	}
	if (i == 0)
		TEST_PASS_MSG("Destination buffer was not modified after the visible area.");
	
	// Free the buffers.
	free(src);
	free(dest);
	putchar('\n');
	
	if (passed)
		return MDP_ERR_OK;
	return -MDP_ERR_UNKNOWN;
}
