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

#if !defined(GENS_DEBUG)
// Signal handler.
#include "sighandler.h"
#endif

#include "md_palette.hpp"
#include "gens_ui.hpp"
#include "g_md.hpp"

// Command line parsing.
#include "parse.hpp"

#include "util/file/config_file.hpp"
#include "libgsft/gsft_szprintf.h"

// Video, Input.
#include "video/vdraw_haiku_t.h"
#include "video/vdraw_cpp.hpp"
#include "input/input_haiku_t.h"

// Application Related.
#include "ui/gens_ui.hpp"
#include "ui/haiku/gens/gens_app.hpp"
#include "gens/gens_window.h"
#include "gens/gens_window_sync.hpp"
#include "ui/haiku/gens/gens_menu.hpp"

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
	szprintf(buf, size, "%s%s", getenv("HOME"), GENS_DEFAULT_SAVE_PATH);
	
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
#if !defined(GENS_DEBUG)
	// Install the signal handler.
	gens_sighandler_init();
#endif
	
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
	Gens_StartupInfo_t *startup = parse_args(argc, argv);
	
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
	
	// Check the startup mode.
	check_startup_mode(startup);
	free(startup);
	
	// Update the UI.
	GensUI::update();
	
	// Reset the renderer.
	vdraw_reset_renderer(true);
	
	// Synchronize the Gens window.
	Sync_Gens_Window();
	
	// Create the Gens window.
	//gens_window_create();
	new GensApplication();

	// Set the window title to Idle.
	GensUI::setWindowTitle_Idle();
	
	// Enter the main application loop
	be_app->SetPulseRate(10000);
	be_app->Run();
	delete be_app;
	
	// Save the configuration file.
#ifndef PACKAGE_NAME
#error PACKAGE_NAME not defined!
#endif
	string cfg_filename = string(PathNames.Gens_Path) + PACKAGE_NAME + ".cfg";
	Config::save(cfg_filename);
	
	End_All();
	
#if !defined(GENS_DEBUG)
	// Shut down the signal handler.
	gens_sighandler_end();
#endif
	
	return 0;
}
