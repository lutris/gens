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

#include "gens.hpp"
#include "g_main.hpp"
#include "g_main_win32.hpp"

// libgsft includes.
#include "libgsft/gsft_unused.h"
#include "libgsft/gsft_strlcpy.h"
#include "libgsft/gsft_szprintf.h"
#include "libgsft/gsft_win32_gdi.h"

// C++ includes.
#include <string>
#include <list>
using std::string;
using std::list;

#if !defined(GENS_DEBUG)
// Signal handler.
#include "sighandler.h"
#endif

// Win32 Unicode support.
#include "libgsft/w32u/w32u_windows.h"
#include "libgsft/w32u/w32u_charset.h"
#include "libgsft/w32u/w32u_shlobj.h"
#include "libgsft/w32u/w32u_libc.h"

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

// Message logging.
#include "macros/log_msg.h"

// Default save path.
#define GENS_DEFAULT_SAVE_PATH ".\\"


typedef struct _csidl_dir_t
{
	int nFolder;		// CSIDL value. (negative number == end of list)
	const char *env_var;	// Environment variable fallback, if available.
	const char *def;	// Default pathname fallback.
} csidl_dir_t;

/**
 * checkSpecialDirectory(): Check if a pathname contains a Special Directory.
 * @param path Pathname. (If NULL, simply resolves the special directory and doesn't check anything.)
 * @param SpecialDirs Pointer to a special directory.
 * @return Special directory pathname, or empty if none.
 */
static string checkSpecialDirectory(const char *path, const csidl_dir_t *SpecialDir)
{
	// Get the directory.
	LPITEMIDLIST pidl;
	char spdir_buf[MAX_PATH+32];
	
	HRESULT hRet = SHGetSpecialFolderLocation(NULL, SpecialDir->nFolder, &pidl);
	
	spdir_buf[0] = 0x00;
	if (SUCCEEDED(hRet))
	{
		// Directory obtained.
		pSHGetPathFromIDListU(pidl, spdir_buf);
	}
	else if (SpecialDir->env_var != NULL)
	{
		// Directory not obtained.
		// Check the environment variable.
		char *env_var = getenv(SpecialDir->env_var);
		if (env_var)
			strlcpy(spdir_buf, getenv(SpecialDir->env_var), sizeof(spdir_buf));
	}
	else
	{
		// Directory not obtained.
		return "";
	}
	
	if (spdir_buf[0] == 0x00)
	{
		// Empty buffer. Check if there's a default fallback.
		if (!SpecialDir->def)
			return "";
		
		// Default fallback found.
		// TODO: This will trigger a false positive if the
		// Program Files directory is "C:\Program Files",
		// but the program is located in "D:\Program Files".
		strlcpy(spdir_buf, SpecialDir->def, sizeof(spdir_buf));
		spdir_buf[0] = (path ? path[0] : 'C');
	}
	
	spdir_buf[sizeof(spdir_buf)-1] = 0x00;
	size_t len = strlen(spdir_buf);
	if (len == 0)
		return "";
	
	// Make sure the directory name has a trailing backslash.
	if (spdir_buf[len-1] != '\\')
	{
		spdir_buf[len] = '\\';
		spdir_buf[len+1] = 0x00;
		len++;
	}
	
	if (!path)
	{
		// No path specified.
		// Return the directory as-is.
		return string(spdir_buf);
	}
	
	// Check if the directory is the same as the
	// beginning of the path to check.
	if (strncasecmp(path, spdir_buf, len) == 0)
	{
		// Directory is the same!
		return string(spdir_buf);
	}
	
	// Path doesn't match the special directory.
	return "";
}


/**
 * get_default_save_path(): Get the default save path.
 * @param buf Buffer to store the default save path in.
 * @param size Size of the buffer.
 */
void get_default_save_path(char *buf, size_t size)
{
	// Win32 needs the program's pathname.
	pGetModuleFileNameU(NULL, PathNames.Gens_EXE_Path, sizeof(PathNames.Gens_EXE_Path));
	PathNames.Gens_EXE_Path[sizeof(PathNames.Gens_EXE_Path)-1] = 0x00;
	
	// Remove the filename portion of the pathname.
	char *last_backslash = strrchr(PathNames.Gens_EXE_Path, '\\');
	if (last_backslash)
	{
		*(last_backslash + 1) = 0x00;
	}
	
	// Check if the EXE is in a "special" directory.
	
	// System directories.
	static const csidl_dir_t SpecialDirs_System[] =
	{
		{CSIDL_PROGRAM_FILES, "ProgramFiles", "?:\\Program Files\\"},
		//{CSIDL_PROGRAM_FILESX86, "ProgramFiles(x86)", ":\\Program Files (x86)\\"},
		{CSIDL_WINDOWS, "WINDIR", NULL},
		
		{-1, NULL, NULL}
	};
	
	// Application Data directories.
	static const csidl_dir_t SpecialDirs_AppData[] =
	{
		{CSIDL_APPDATA, "APPDATA", NULL},
		{CSIDL_PROFILE, "USERPROFILE", NULL},
		
		{-1, NULL, NULL}
	};
	
	// Check "Program Files" on all systems.
	string SpecialDir = checkSpecialDirectory(PathNames.Gens_EXE_Path, &SpecialDirs_System[0]);
	if (SpecialDir.empty() && (winVersion.dwPlatformId == VER_PLATFORM_WIN32_NT))
	{
		// Check the Windows directory on NT only.
		// (Windows 9x stores user files in C:\WINDOWS.)
		SpecialDir = checkSpecialDirectory(PathNames.Gens_EXE_Path, &SpecialDirs_System[1]);
	}
	
	// If true, then Gens/GS is in a special directory.
	bool isSpecialDir = !SpecialDir.empty();
	if (isSpecialDir)
	{
		// Special Directory.
		// On Windows 9x and NT 4.0, user profiles are stored in
		// C:\WINDOWS\Profiles. Make sure the user's profile
		// isn't part of the directory.
		string UserProfile = checkSpecialDirectory(PathNames.Gens_EXE_Path, &SpecialDirs_AppData[1]);
		if (!UserProfile.empty())
		{
			// The path is part of the user profile.
			// Gens/GS is not located in a special directory.
			isSpecialDir = false;
		}
	}
	
	if (!isSpecialDir)
	{
		// Gens/GS is not located in a special directory.
		// Attempt to create a temporary file.
		// (Checking the directory always returns 0 for some reason.)
		char tmp_buf[1024];
		szprintf(tmp_buf, sizeof(tmp_buf), "%stmpXXXXXX", PathNames.Gens_EXE_Path);
		mktemp(tmp_buf);
		
		FILE *f_tmp = fopen(tmp_buf, "wb");
		if (f_tmp)
		{
			// Temporary file created.
			// Gens/GS directory is writable.
			fclose(f_tmp);
			unlink(tmp_buf);
			
			// Use the current directory as the save path.
			pSetCurrentDirectoryU(PathNames.Gens_EXE_Path);
			strlcpy(buf, PathNames.Gens_EXE_Path, size);
			return;
		}
	}
	
	// Gens/GS is located in a special directory.
	// Get the "Application Data" directory.
	SpecialDir = checkSpecialDirectory(NULL, &SpecialDirs_AppData[0]);
	if (SpecialDir.empty())
	{
		// "Application Data" directory not found.
		// Get the user profile directory.
		SpecialDir = checkSpecialDirectory(NULL, &SpecialDirs_AppData[1]);
		if (SpecialDir.empty())
		{
			// User profile directory not found.
			// TODO: For now, simply fall back to the current directory.
			pSetCurrentDirectoryU(PathNames.Gens_EXE_Path);
			strlcpy(buf, PathNames.Gens_EXE_Path, size);
			return;
		}
		
		// Check if "Application Data" exists as a subdirectory.
		// NOTE: Win32's stat() doesn't like trailing slashes.
		string AppDataSubDir = SpecialDir + "Application Data";
		
		struct stat st_appdata;
		int ret = stat(AppDataSubDir.c_str(), &st_appdata);
		if (ret == 0 && S_ISDIR(st_appdata.st_mode))
		{
			// Subdirectory exists.
			SpecialDir = AppDataSubDir + "\\";
		}
	}
	
	// Append "Gens\\" to the subdirectory.
	SpecialDir += "Gens\\";
	
	// Make sure the Gens subdirectory exists.
	mkdir(SpecialDir.c_str());
	
	// Return the Gens subdirectory.
	pSetCurrentDirectoryU(SpecialDir.c_str());
	strlcpy(buf, SpecialDir.c_str(), size);
}


static UINT cp_orig = CP_ACP;
#define SET_CONSOLE_OUTPUT_CP_UTF8() \
do { \
	cp_orig = GetConsoleOutputCP(); \
	SetConsoleOutputCP(CP_UTF8); \
} while (0)
#define SET_CONSOLE_OUTPUT_CP_ACP() \
	SetConsoleOutputCP(cp_orig)


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
	
	// Initialize the Win32 Unicode Translation Layer.
	int ret = w32u_init();
	switch (ret)
	{
		case ERR_W32U_SUCCESS:
			// Initialized successfully.
			break;
		
		case -ERR_W32U_UTF8_NOT_SUPPORTED:
			// UTF-8 isn't supported.
			MessageBoxA(NULL, GENS_APPNAME " requires a system that support UTF-8.\n"
					"This system does not support it.\n\n"
					"Consider upgrading to an operating system released\n"
					"in the past ten years or so.",
					GENS_APPNAME ": UTF-8 Required", MB_ICONSTOP);
			TerminateProcess(GetCurrentProcess(), -1);
			return -1;
		
		default:
		{
			// Unknown error.
			char buf[2048];
			szprintf(buf, sizeof(buf), "An unknown error has occurred while initializing\n"
					"the Win32 Unicode Translation Layer.\n\n"
					"Error code: %d\n", ret);
			MessageBoxA(NULL, buf, GENS_APPNAME ": Unicode Error", MB_ICONSTOP);
			TerminateProcess(GetCurrentProcess(), -2);
			return -2;
		}
	}
	
#if defined(GENS_WIN32_CONSOLE)
	SET_CONSOLE_OUTPUT_CP_UTF8();
#endif
	
	// Get the Windows version.
	winVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	if (pGetVersionExU((OSVERSIONINFO*)(&winVersion)) == 0)
	{
		winVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		pGetVersionExU((OSVERSIONINFO*)(&winVersion));
	}
	
	// Initialize the PRNG.
	Init_PRNG();
	
	// gens_window is needed before anything else is set up.
	// Initialize the Gens hWnd.
	// (NOTE: Fonts are now initialized in gens_window_init_hWnd().)
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
	Gens_StartupInfo_t *startup;
	argc_argv arg;
	char *lpuCmdLine = NULL;
	bool bNeedsPrgName;		// If true, convertCmdLineToArgv() will add the program name as arg.v[0].
	
	if (w32u_is_unicode)
	{
		// Unicode version.
		// NOTE: lpwCmdLine *does* contain the program name, whereas lpCmdLine does not.
		bNeedsPrgName = false;
		const wchar_t *lpwCmdLine = GetCommandLineW();
		lpuCmdLine = w32u_UTF16toUTF8(lpwCmdLine);
	}
	else
	{
		// ANSI version.
		// NOTE: lpCmdLine does not contain the program name.
		bNeedsPrgName = true;
		lpuCmdLine = w32u_ANSItoUTF8(lpCmdLine);
	}
	
	if (lpuCmdLine)
	{
		// Command line converted to UTF-8 successfully.
		// Parse the command line arguments.
		convertCmdLineToArgv(lpuCmdLine, &arg, bNeedsPrgName);
		startup = parse_args(arg.c, arg.v);
		free(lpuCmdLine);
	}
	else
	{
		// Couldn't convert the command line to UTF-8.
		// Create a blank startup struct.
		startup = (Gens_StartupInfo_t*)malloc(sizeof(Gens_StartupInfo_t));
		memset(startup, 0x00, sizeof(Gens_StartupInfo_t));
	}
	
	// Delete the command line arguments.
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
	int enable_debug_console = startup->enable_debug_console;
	if (enable_debug_console)
	{
		// Allocate a console.
		// Example code from http://justcheckingonall.wordpress.com/2008/08/29/console-window-win32-app/
		AllocConsole();
		SET_CONSOLE_OUTPUT_CP_UTF8();
		
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
	}
#endif

#if !defined(GENS_DEBUG)
	// Install the signal handler.
	gens_sighandler_init();	
#endif
	
	// Reset the renderer.
	// This should be done before initializing the backend
	// to make sure vdraw_rInfo is initialized.
	vdraw_reset_renderer(true);
	
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
	string cfg_filename = string(PathNames.Gens_Save_Path) + PACKAGE_NAME + ".cfg";
	Config::save(cfg_filename);
	
	End_All();
	DestroyWindow(gens_window);
	
	// Shut down libgsft_win32_gdi. (Fonts)
	gsft_win32_gdi_end();
	
	// Empty the message queue.
	// NOTE: ANSI functions are used here, since the messages aren't actually processed.
	MSG msg;
	while (PeekMessageA(&msg, NULL, 0, 0, PM_NOREMOVE))
	{
		if (!GetMessageA(&msg, NULL, 0, 0))
			return msg.wParam;
	}
	
#if !defined(GENS_DEBUG)
	// Shut down the signal handler.
	gens_sighandler_end();
#endif
	
	// Reset the console code page.
#if defined(GENS_WIN32_CONSOLE)
	SET_CONSOLE_OUTPUT_CP_ACP();
#else
	if (enable_debug_console)
		SET_CONSOLE_OUTPUT_CP_ACP();
#endif
	
	// Shut down the Win32 Unicode Translation Layer.
	w32u_end();
	
	TerminateProcess(GetCurrentProcess(), 0); //Modif N
	return 0;
}
