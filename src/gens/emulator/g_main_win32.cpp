/***************************************************************************
 * Gens: Main Loop. (Win32-specific code)                                  *
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "g_main.hpp"
#include "g_main_win32.hpp"

// libgsft includes.
#include "libgsft/gsft_unused.h"
#include "libgsft/gsft_strlcpy.h"

// C++ includes.
#include <string>
#include <list>
using std::string;
using std::list;

#if !defined(GENS_DEBUG)
// Signal handler.
#include "sighandler.h"
#endif

#if !defined(GENS_WIN32_CONSOLE)
// Win32 I/O functions. (Required for console allocation.)
#include <io.h>
#include <fcntl.h>
#endif

#include "md_palette.hpp"
#include "gens_ui.hpp"
#include "g_md.hpp"

// Command line parsing.
#include "parse.hpp"
#include "port/argc_argv.h"

#include "util/file/config_file.hpp"

// Video, Input, Audio.
#include "video/vdraw.h"
#include "input/input.h"
#include "audio/audio.h"

// VDraw C++ functions.
#include "video/vdraw_cpp.hpp"

#include "gens/gens_window.h"
#include "gens/gens_window_sync.hpp"

// Win32 instance
HINSTANCE ghInstance = NULL;

// Windows version
OSVERSIONINFOEX winVersion;

// If extended Common Controls are enabled, this is set to a non-zero value.
int win32_CommCtrlEx = 0;

// Fonts.
#include "ui/win32/fonts.h"

// Win32 includes.
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <windowsx.h>

// Message logging.
#include "macros/log_msg.h"

// Default save path.
#define GENS_DEFAULT_SAVE_PATH ".\\"


/**
 * get_default_save_path(): Get the default save path.
 * @param buf Buffer to store the default save path in.
 * @param size Size of the buffer.
 */
void get_default_save_path(char *buf, size_t size)
{
	// Win32 needs the program's pathname.
	GetModuleFileName(NULL, PathNames.Gens_EXE_Path, sizeof(PathNames.Gens_EXE_Path));
	PathNames.Gens_EXE_Path[sizeof(PathNames.Gens_EXE_Path)-1] = 0x00;
	
	// Remove the filename portion of the pathname. (_tcsrchr() == strrchr())
	TCHAR *last_backslash = _tcsrchr(PathNames.Gens_EXE_Path, '\\');
	if (last_backslash)
	{
		*(last_backslash + 1) = 0x00;
	}
	
	// Set the current directory.
	SetCurrentDirectory(PathNames.Gens_EXE_Path);
	
	// Set the default save path.
	strlcpy(buf, GENS_DEFAULT_SAVE_PATH, size);
}


/**
 * WinMain: Win32 main loop.
 * @param hInst Instance ID of the Gens process.
 * @param hPrevInst Instance ID of the previous Gens process. (Win16 only.)
 * @param lpCmdLine Command line.
 * @param nCmdShow How to show the Gens window.
 * @return Error code.
 */
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
	// hPrevInst is a leftover from Win16.
	GSFT_UNUSED_PARAMETER(hPrevInst);
	
	// Save hInst for other functions.
	ghInstance = hInst;
	
	// Initialize the PRNG.
	Init_PRNG();
	
	// Initialize fonts.
	fonts_init();
	
	// gens_window is needed before anything else is set up.
	// Initialize the Gens hWnd.
	gens_window_init_hWnd();
	
	// Initialize vdraw_ddraw.
	vdraw_init();
	// TODO: This doesn't seem to be needed on Win32...
	//vdraw_backend_init_subsystem(VDRAW_BACKEND_DDRAW);
	
	// Initialize the input object.
	int rval = input_init(INPUT_BACKEND_DINPUT);
	if (rval != 0)
	{
		// DirectInput could not be initialized for some reason.
		exit(rval);
	}
	
	// Initialize the Settings struct.
	if (Init_Settings())
	{
		// Error initializing settings.
		input_end();
		vdraw_end();
		return 1;	// TODO: Replace with a better error code.
	}
	
	// Parse command line arguments.
	argc_argv arg;
	Gens_StartupInfo_t *startup;
	
	convertCmdLineToArgv(lpCmdLine, &arg);
	startup = parse_args(arg.c, arg.v);
	deleteArgcArgv(&arg);
	
	// Recalculate the palettes, in case a command line argument changed a video setting.
	Recalculate_Palettes();
	
	Init_Genesis_Bios();
	
	// Initialize Gens.
	if (!Init())
		return 0;
	
	// Initialize the UI.
	GensUI::init(NULL, NULL);
	
	// not yet finished (? - wryun)
	//initializeConsoleRomsView();
	
#if !defined(GENS_WIN32_CONSOLE)
	if (startup->enable_debug_console)
	{
		// Allocate a console.
		// Example code from http://justcheckingonall.wordpress.com/2008/08/29/console-window-win32-app/
		AllocConsole();
		
		HANDLE handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
		int hCrt = _open_osfhandle((long)handle_out, _O_TEXT);
		FILE *hf_out = _fdopen(hCrt, "w");
		setvbuf(hf_out, NULL, _IONBF, 1);
		*stdout = *hf_out;
		*stderr = *hf_out;
		
		HANDLE handle_in = GetStdHandle(STD_INPUT_HANDLE);
		hCrt = _open_osfhandle((long)handle_in, _O_TEXT);
		FILE *hf_in = _fdopen(hCrt, "r");
		setvbuf(hf_in, NULL, _IONBF, 128);
		*stdin = *hf_in;
		
#if !defined(GENS_DEBUG)
		// Install the signal handler.
		gens_sighandler_init();
#endif
	}
#endif
	
	// Initialize the video backend.
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
	
	// Show the Gens window.
	ShowWindow(gens_window, nCmdShow);
	SetFocus(gens_window);
	
	// Run the Gens Main Loop.
	GensMainLoop();
	
	audio_clear_sound_buffer();
	
	// Save the configuration file.
#ifndef PACKAGE_NAME
#error PACKAGE_NAME not defined!
#endif
	string cfg_filename = string(PathNames.Gens_Path) + PACKAGE_NAME + ".cfg";
	Config::save(cfg_filename);
	
	End_All();
	DestroyWindow(gens_window);
	
	// Delete the fonts.
	fonts_end();
	
	// Empty the message queue.
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
	{
		if (!GetMessage(&msg, NULL, 0, 0))
			return msg.wParam;
	}
	
#if !defined(GENS_DEBUG)
	// Shut down the signal handler.
	gens_sighandler_end();
#endif
	
	TerminateProcess(GetCurrentProcess(), 0); //Modif N
	return 0;
}
