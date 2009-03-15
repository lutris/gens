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

#include "g_main.hpp"
#include "g_main_win32.hpp"

#include "g_palette.h"
#include "gens_ui.hpp"
#include "parse.hpp"

#include "g_md.hpp"
#include "g_mcd.hpp"
#include "g_32x.hpp"

#include "gens_core/vdp/vdp_io.h"
#include "util/file/config_file.hpp"
#include "util/sound/gym.hpp"

#include "video/vdraw.h"
#include "input/input.h"
#include "audio/audio.h"

// VDraw C++ functions.
#include "video/vdraw_cpp.hpp"

#include "gens/gens_window.h"
#include "gens/gens_window_sync.hpp"

#ifdef GENS_DEBUGGER
#include "debugger/debugger.hpp"
#endif /* GENS_DEBUGGER */

// Win32 instance
HINSTANCE ghInstance = NULL;

// Windows version
OSVERSIONINFO winVersion;

// If extended Common Controls are enabled, this is set to a non-zero value.
int win32_CommCtrlEx = 0;

// Fonts.
#include "ui/win32/fonts.h"

// min() and max().
#include <minmax.h>

// Windows macros.
#include <windowsx.h>

// argc/argv conversion.
#include "port/argc_argv.h"

// C++ includes
#include <list>
using std::list;


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
	
	// Set the current directory.
	SetCurrentDirectory(PathNames.Gens_EXE_Path);
	
	// Eliminate the filename portion of the pathname.
	char *last_backslash = strrchr(PathNames.Gens_EXE_Path, '\\');
	if (last_backslash)
	{
		*(last_backslash + 1) = 0x00;
	}
	
	// Set the default save path.
	strncpy(buf, GENS_DEFAULT_SAVE_PATH, size);
	buf[size-1] = 0x00;
}


/**
 * Win32_centerOnGensWindow(): Center the specified window on the Gens window.
 * @param hWnd Window handle.
 */
void Win32_centerOnGensWindow(HWND hWnd)
{
	// TODO: Move this to GensUI.
	
	RECT r, r2;
	int dx1, dy1, dx2, dy2;
	
	GetWindowRect(gens_window, &r);
	dx1 = (r.right - r.left) / 2;
	dy1 = (r.bottom - r.top) / 2;
	
	GetWindowRect(hWnd, &r2);
	dx2 = (r2.right - r2.left) / 2;
	dy2 = (r2.bottom - r2.top) / 2;
	
	SetWindowPos(hWnd, NULL,
		     max(0, r.left + (dx1 - dx2)),
		     max(0, r.top + (dy1 - dy2)), 0, 0,
		     SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}


/**
 * Win32_setActualWindowSize(): Set the actual window size, including the non-client area.
 * @param hWnd Window handle.
 * @param reqW Required width.
 * @param reqH Required height.
 */
void Win32_setActualWindowSize(HWND hWnd, const int reqW, const int reqH)
{
	RECT r;
	SetRect(&r, 0, 0, reqW, reqH);
	
	// Adjust the rectangle.
	AdjustWindowRectEx(&r, GetWindowStyle(hWnd), (GetMenu(hWnd) != NULL), GetWindowExStyle(hWnd));
	
	// Set the window size.
	SetWindowPos(hWnd, NULL, 0, 0, r.right - r.left, r.bottom - r.top,
		     SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
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
	// Save hInst for other functions.
	ghInstance = hInst;
	
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
	convertCmdLineToArgv(lpCmdLine, &arg);
	parse_args(arg.c, arg.v);
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
			fprintf(stderr, "%s(): Failed to load %s\n", __func__, PathNames.Start_Rom);
		}
	}
	
	// Update the UI.
	GensUI::update();
	
	// Reset the renderer.
	vdraw_reset_renderer(TRUE);
	
	// Synchronize the Gens window.
	Sync_Gens_Window();
	
	// Show the Gens window.
	ShowWindow(gens_window, nCmdShow);
	
	// Run the Gens Main Loop.
	GensMainLoop();
	
	audio_clear_sound_buffer();
	
	// Save the configuration file.
	char save_path[GENS_PATH_MAX];
	get_default_save_path(save_path, sizeof(save_path));
	strcat(save_path, "gens.cfg");
	Config::save(save_path);
	
	End_All();
	ChangeDisplaySettings(NULL, 0);
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
	
	TerminateProcess(GetCurrentProcess(), 0); //Modif N
	
	return 0;
}
