/***************************************************************************
 * Gens: Main Loop. (Haiku-specific code)                                  *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2009 by David Korth                                  *
 * Copyright (c) 2009 by Phil Costin                                       *
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "g_main.hpp"
#include "g_main_haiku.hpp"

// C includes.
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// C++ includes
#include <list>
using std::list;

// Signal handler.
#include "sighandler.h"

#include "md_palette.hpp"
#include "gens_ui.hpp"
#include "g_md.hpp"

// Command line parsing.
#include "parse.hpp"

#include "util/file/config_file.hpp"

// Video, Input.
#include "video/vdraw_sdl.h"
#include "video/vdraw_cpp.hpp"
#include "input/input_sdl.h"

#include "gens/gens_window.h"
#include "gens/gens_window_sync.hpp"

#include "port/timer.h"

// Message logging.
#include "macros/log_msg.h"

// Default save path.
#define GENS_DEFAULT_SAVE_PATH "/.gens/"


/**
 * get_default_save_path(): Get the default save path.
 * @param buf Buffer to store the default save path in.
 * @param size Size of the buffer.
 */
void get_default_save_path(char *buf, size_t size)
{
	// Prepend the user's home directory to the save path.
	snprintf(buf, size, "%s%s", getenv("HOME"), GENS_DEFAULT_SAVE_PATH);
	
	// Make sure the buffer is null-terminated.
	buf[size-1] = 0x00;
	
	// Make sure the directory exists.
	// NOTE: 0777 is used to allow for custom umask settings.
	mkdir(buf, 0777);
}


/**
 * main(): Main loop.
 * @param argc Number of arguments.
 * @param argv Array of arguments.
 * @return Error code.
 */
int main(int argc, char *argv[])
{
	if (geteuid() == 0)
	{
		// Don't run Gens/GS as root!
		static const char gensRootErr[] =
				"Error: Gens/GS should not be run as root.\n"
				"Please log in as a regular user.";
		
		fprintf(stderr, "%s\n", gensRootErr);
		
		return 1;
	}
	
	// Install the signal handler.
	gens_sighandler_init();
	
	// Initialize the timer.
	// TODO: Make this unnecessary.
	init_timer();
	
	// Initialize the UI.
	GensUI::init(&argc, &argv);
	
	// Initialize vdraw_sdl.
	vdraw_init();
	vdraw_backend_init_subsystem(VDRAW_BACKEND_HAIKU);
	
	// Initialize input_sdl.
	input_init(INPUT_BACKEND_HAIKU);
	
	// Initialize the Settings struct.
	if (Init_Settings())
	{
		// Error initializing settings.
		input_end();
		vdraw_end();
		return 1;	// TODO: Replace with a better error code.
	}
	
	// Parse command line arguments.
	parse_args(argc, argv);
	
	// Recalculate the palettes, in case a command line argument changed a video setting.
	Recalculate_Palettes();
	
	Init_Genesis_Bios();
	
	// Initialize Gens.
	if (!Init())
		return 0;
	
	// not yet finished (? - wryun)
	//initializeConsoleRomsView();
	
	// Initialize the backend.
	if ((int)vdraw_cur_backend_id < 0)
	{
		// No backend saved in the configuration file. Use the default.
		vdraw_backend_init((VDRAW_BACKEND)0);
	}
	else
	{
		// Backend saved in the configuration file. Use that backend.
		vdraw_backend_init(vdraw_cur_backend_id);
	}
	
	if (strcmp(PathNames.Start_Rom, "") != 0)
	{
		if (ROM::openROM(PathNames.Start_Rom) == -1)
		{
			// Could not open the startup ROM.
			// TODO: Show a message box?
			LOG_MSG(gens, LOG_MSG_LEVEL_ERROR,
				"Failed to load ROM '%s'.", PathNames.Start_Rom);
		}
	}
	
	// Update the UI.
	GensUI::update();
	
	// Reset the renderer.
	vdraw_reset_renderer(true);
	
	// Synchronize the Gens window.
	Sync_Gens_Window();
	
	// Show the Gens window.
	//gtk_widget_show(gens_window);
	
	// Run the Gens Main Loop.
	GensMainLoop();
	
	// Save the configuration file.
	char save_path[GENS_PATH_MAX];
	get_default_save_path(save_path, sizeof(save_path));
	strcat(save_path, "gens.cfg");
	Config::save(save_path);
	
	End_All();
	
	// Shut down the signal handler.
	gens_sighandler_end();
	
	return 0;
}
