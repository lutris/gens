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

// C includes.
#include <stdio.h>
#include <stdlib.h>

// MDP includes.
#include "mdp/mdp_dlopen.h"
#include "mdp/mdp_error.h"


/**
 * plugin_check(): Open a plugin and test it.
 * @param filename Plugin filename.
 * @return 0 if the plugin passes all tests; non-zero on error.
 */
int plugin_check(const char *filename)
{
	int rval = 0;
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
	void *mdp = mdp_dlsym(DLL, "mdp");
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
	
Finish:
	if (DLL)
		mdp_dlclose(DLL);
	printf("Finished testing plugin '%s'.\n", filename);
	return rval;
}
