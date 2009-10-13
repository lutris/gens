/***************************************************************************
 * MDP: Mega Drive Plugins - Test Suite.                                   *
 * main.c: Entry Point.                                                    *
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

#include "main.h"
#include "plugin_check.h"

// C includes.
#include <stdio.h>
#include <stdlib.h>

// libgsft includes.
#include "libgsft/gsft_szprintf.h"
#include "libgsft/gsft_strlcpy.h"

int main(int argc, char *argv[])
{
	fprintf(stderr, "MDP: Mega Drive Plugins %d.%d.%d\n"
		"Test Suite version %d.%d.%d%s\n\n"
		"Copyright (c) 2008-2009 by David Korth.\n\n"
		"This program is licensed under the GNU General Public License v2.\n"
		"See http://www.gnu.org/licenses/gpl-2.0.html for more information.\n\n",
		MDP_VERSION_MAJOR(TEST_MDP_INTERFACE_VERSION),
		MDP_VERSION_MINOR(TEST_MDP_INTERFACE_VERSION),
		MDP_VERSION_REVISION(TEST_MDP_INTERFACE_VERSION),
		MDP_VERSION_MAJOR(TEST_VERSION),
		MDP_VERSION_MINOR(TEST_VERSION),
		MDP_VERSION_REVISION(TEST_VERSION),
#ifdef TEST_VERSION_DEVEL
		"+"
#else
		""
#endif
	       );
	
	// Check parameters.
	if (argc <= 1)
	{
		// No plugins specified.
		const char *exe_name;
#ifdef _WIN32
		// Only display the filename.
		exe_name = strrchr(argv[0], DIRSEP_CHR);
		if (!exe_name)
			exe_name = argv[0];
		else
			exe_name++;
#else
		// Display the whole argv[0].
		exe_name = argv[0];
#endif
		
		fprintf(stderr, "Usage: %s filename [filename...]\n", exe_name);
		return EXIT_FAILURE;
	}
	
	// Check all plugins.
	int i;
	int success_count = 0;
	char dll_filename[PATH_MAX];
	for (i = 1; i < argc; i++)
	{
#ifndef _WIN32
		// If a relative pathname is specified, add "./" to it.
		if ((argv[i][0] != DIRSEP_CHR) &&
		    (argv[i][0] != '.' && argv[i][1] != DIRSEP_CHR) &&
		    (argv[i][0] != '.' && argv[i][1] != '.' && argv[i][2] != DIRSEP_CHR))
		{
			szprintf(dll_filename, sizeof(dll_filename),
				 ".%c%s", DIRSEP_CHR, argv[i]);
		}
		else
#endif
		{
			strlcpy(dll_filename, argv[i], sizeof(dll_filename));
		}
		
		if (!plugin_check(dll_filename))
			success_count++;
		
		// Add a newline between plugin checks.
		putchar('\n');
	}
	
	printf("%d/%d plugin%s tested successfully.\n",
	       success_count, argc-1, (success_count == 1 ? "" : "s"));
}
