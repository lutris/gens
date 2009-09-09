/***************************************************************************
 * MDP: Mega Drive Plugins - Test Suite.                                   *
 * plugin_init.c: Plugin Check Function.                                   *
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

#include "plugin_check.h"
#include "main.h"

#include "host_srv.h"
#include "render.hpp"

// C includes.
#include <stdio.h>
#include <stdlib.h>

// MDP includes.
#include "mdp/mdp_dlopen.h"
#include "mdp/mdp_error.h"


static int validate_plugin_info(const mdp_t *plugin);
static int initialize_plugin(const mdp_t *plugin);
static int shutdown_plugin(const mdp_t *plugin);


/**
 * plugin_check(): Open a plugin and test it.
 * @param filename Plugin filename.
 * @return 0 if the plugin passes all tests; non-zero on error.
 */
int plugin_check(const char *filename)
{
	int rval = 0;
	int passed = 1;
	printf("Testing plugin '%s'...\n", filename);
	
	// Attempt to open the specified plugin.
	TEST_START("mdp_dlopen()");
	void *DLL = mdp_dlopen(filename);
	if (!DLL)
	{
		// An error occurred while trying to open the plugin.
		const char *err = mdp_dlerror();
		TEST_FAIL(err);
		mdp_dlerror_str_free(err);
		
		rval = -MDP_ERR_CANNOT_OPEN_DLL;
		goto Finish;
	}
	TEST_PASS();
	
	// Get the MDP symbol.
	TEST_START("mdp_dlsym()");
	const mdp_t *mdp = (mdp_t*)mdp_dlsym(DLL, "mdp");
	if (!mdp)
	{
		// Could not get the MDP symbol.
		const char *err = mdp_dlerror();
		TEST_FAIL(err);
		mdp_dlerror_str_free(err);
		
		rval = -MDP_ERR_NO_MDP_SYMBOL;
		goto Finish;
	}
	TEST_PASS();
	
	// Validate the MDP plugin information struct.
	rval = validate_plugin_info(mdp);
	if (rval != 0)
		goto Finish;
	
	// Initialize the plugin.
	rval = initialize_plugin(mdp);
	if (rval != 0)
		goto Finish;
	
	// Test renderer plugins.
	rval = renderer_test_all();
	if (rval != 0)
		passed = 0;
	
	// Shut down the plugin.
	rval = shutdown_plugin(mdp);
	if (rval != 0)
		goto Finish;
	
Finish:
	if (DLL)
		mdp_dlclose(DLL);
	printf("Finished testing plugin '%s'.\n", filename);
	if (passed == 1)
		return rval;
	return -MDP_ERR_UNKNOWN;
}


/**
 * validate_plugin_info(): Validate the plugin information struct.
 * @param mdp Plugin information struct.
 * @return MDP error code.
 */
static int validate_plugin_info(const mdp_t *plugin)
{
	TEST_START("Validating plugin information struct");
	
	// Check the mdp_t version.
	if (MDP_VERSION_MAJOR(plugin->interfaceVersion) !=
	    MDP_VERSION_MAJOR(TEST_MDP_INTERFACE_VERSION))
	{
		// Incorrect major version.
		TEST_FAIL_ARGS("Plugin major version mismatch. (mdp_test == %d; plugin == %d)",
			       MDP_VERSION_MAJOR(TEST_MDP_INTERFACE_VERSION),
			       MDP_VERSION_MAJOR(plugin->interfaceVersion));
		return -MDP_ERR_INCORRECT_MAJOR_VERSION;
	}
	
	// Add a newline.
	putchar('\n');
	
	// Check for a plugin description field.
	mdp_desc_t *desc = plugin->desc;
	if (desc)
	{
		TEST_INFO("Plugin Description:");
		
		// Name.
		if (desc->name)
			TEST_INFO_ARGS("Name: %s", desc->name);
		else
			TEST_WARN("mdp->desc->name is NULL.");
		
		// MDP author.
		if (desc->author_mdp)
			TEST_INFO_ARGS("MDP Author: %s", desc->author_mdp);
		else
			TEST_WARN("mdp->desc->author_mdp is NULL.");
		
		// Original author.
		if (desc->author_orig)
			TEST_INFO_ARGS("Original Author: %s", desc->author_orig);
		
		// Website.
		if (desc->website)
			TEST_INFO_ARGS("Website: %s", desc->website);
		
		// License.
		if (desc->license)
			TEST_INFO_ARGS("License: %s", desc->license);
		else
			TEST_WARN("mdp->desc->license is NULL.");
		
		// Description.
		if (desc->description)
			TEST_INFO_ARGS("Description: %s", desc->description);
		else
			TEST_WARN("mdp->desc->description is NULL.");
	}
	else
	{
		// No plugin description field.
		TEST_WARN("Plugin description field is NULL.");
	}
	
	// UUID.
	const unsigned char *uuid = plugin->uuid;
	TEST_INFO_ARGS("UUID: %02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
		       uuid[0], uuid[1], uuid[2], uuid[3],
		       uuid[4], uuid[5],
		       uuid[6], uuid[7],
		       uuid[8], uuid[9],
		       uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]);
	
	// CPU flags.
	// TODO: Print the symbolic representations of the CPU flags.
	// TODO: Don't allow testing of plugins that require CPU flags not found on this system.
	TEST_INFO("CPU flags:");
	TEST_INFO_ARGS("\tSupported == 0x%08X", plugin->cpuFlagsSupported);
	TEST_INFO_ARGS("\tRequired  == 0x%08X", plugin->cpuFlagsRequired);
	
	TEST_PASS();
	return MDP_ERR_OK;
}


/**
 * initialize_plugin(): Initialize the plugin.
 * @param mdp Plugin information struct.
 * @return MDP error code.
 */
static int initialize_plugin(const mdp_t *plugin)
{
	TEST_START("Initializing plugin");
	putchar('\n');
	
	// Get the mdp_func_t pointer.
	mdp_func_t *func = plugin->func;
	if (!func)
	{
		TEST_FAIL("mdp->func is NULL.");
		return -MDP_ERR_FUNCTION_NOT_IMPLEMENTED;
	}
	
	// Check that mdp->func->init isn't NULL.
	if (!func->init)
	{
		TEST_FAIL("mdp->func->init is NULL.");
		return -MDP_ERR_FUNCTION_NOT_IMPLEMENTED;
	}
	
	// Attempt to initialize the plugin.
	int rval = func->init(&host_srv);
	if (rval != 0)
	{
		// Error in plugin initialization.
		TEST_FAIL_MDP(rval);
	}
	else
	{
		// Plugin initialized successfully.
		TEST_PASS();
	}
	
	return rval;
}


/**
 * shutdown_plugin(): Shut down the plugin.
 * @param mdp Plugin information struct.
 * @return MDP error code.
 */
static int shutdown_plugin(const mdp_t *plugin)
{
	TEST_START("Shutting down plugin");
	putchar('\n');
	
	// Get the mdp_func_t pointer.
	mdp_func_t *func = plugin->func;
	if (!func)
	{
		TEST_FAIL("mdp->func is NULL.");
		return -MDP_ERR_FUNCTION_NOT_IMPLEMENTED;
	}
	
	// Check that mdp->func->end isn't NULL.
	if (!func->end)
	{
		TEST_FAIL("mdp->func->end is NULL.");
		return -MDP_ERR_FUNCTION_NOT_IMPLEMENTED;
	}
	
	// Attempt to shut down the plugin.
	int rval = func->end();
	if (rval != 0)
	{
		// Error in plugin shutdown.
		TEST_FAIL_MDP(rval);
	}
	
	// Clear the renderer plugin list.
	const int plsize = renderer_clear();
	if (rval == 0)
	{
		if (plsize == 0)
		{
			// Plugin shut down successfully.
			TEST_PASS();
		}
		else
		{
			// Indicate an error.
			// TODO: Add a specific error code.
			rval = -MDP_ERR_RENDER_UNKNOWN;
		}
	}
	
	return rval;
}
