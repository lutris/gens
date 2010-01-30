/***************************************************************************
 * libgsft_w32u: Win32 Unicode Translation Layer.                          *
 * w32u.c: Main Unicode translation code.                                  *
 *                                                                         *
 * Copyright (c) 2009 by David Korth.                                      *
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

#include "w32u.h"
#include "w32u_windows.h"
#include "w32u_windowsx.h"
#include "w32u_commctrl.h"
#include "w32u_shellapi.h"
#include "w32u_libc.h"
#include "w32u_commdlg.h"
#include "w32u_shlobj.h"
#include "w32u_winnls.h"

// shlwapi.h needed for DLLVERSIONINFO and DLLGETVERSIONPROC.
#include <shlwapi.h>

// C includes.
#include <string.h>
#include <stdlib.h>
#include <wchar.h>

// Initialization counter.
static int init_counter = 0;

// Is this Unicode?
BOOL w32u_is_unicode = 0;

/** DLL versions. (0xMMNNRRRR) **/
DWORD comctl32_dll_version = 0;
DWORD shell32_dll_version = 0;


/**
 * GetDllVersionNumber(): Get a DLL's version number via DllGetVersion().
 * @param filename DLL filename.
 * @return DLL version number, or 0 on error.
 */
DWORD WINAPI GetDllVersionNumber(const char *filename)
{
	HMODULE hDll = LoadLibrary(filename);
	if (!hDll)
		return 0;
	
	DLLGETVERSIONPROC dllviproc;
	dllviproc = (DLLGETVERSIONPROC)GetProcAddress(hDll, "DllGetVersion");
	if (!dllviproc)
		return 0;
	
	// DllGetVersion() found.
	DLLVERSIONINFO dllvi;
	dllvi.cbSize = sizeof(dllvi);
	
	HRESULT hRet = dllviproc(&dllvi);
	if (hRet != S_OK)
		return 0;
	
	// Received the version number.
	FreeLibrary(hDll);
	
	// Return the version number.
	return  (dllvi.dwMajorVersion & 0xFF) << 24 |
		(dllvi.dwMinorVersion & 0xFF) << 16 |
		(dllvi.dwBuildNumber & 0xFFFF);
}


/**
 * w32u_check_UTF8(): Check if UTF-8 is supported.
 * @return 0 if UTF-8 is supported; non-zero if it isn't.
 */
static int WINAPI w32u_check_UTF8(void)
{
	static const char s_utf8_test[] =
	{
		0xC2, 0xA5, 0xC3, 0xB3, 0xC3, 0xB9, 0x20, 0x6A, 0xC3, 0xBC, 0xC5, 0x9F, 0x74, 0x20, 0xC2, 0xA3,
		0xC3, 0xB5, 0xC5, 0x9F, 0xE2, 0x94, 0xBC, 0x20, 0x54, 0xE1, 0xB8, 0xA9, 0xE1, 0xBA, 0xBD, 0x20,
		0xC4, 0xA2, 0xC3, 0xA2, 0x6D, 0xC4, 0x95, 0x2E, 0x20, 0xE2, 0x8C, 0x98, 0x00
	};
	static const wchar_t s_utf16_test[] =
	{
		0x00A5, 0x00F3, 0x00F9, 0x0020, 0x006A, 0x00FC, 0x015F, 0x0074, 0x0020, 0x00A3,
		0x00F5, 0x015F, 0x253C, 0x0020, 0x0054, 0x1E29, 0x1EBD, 0x0020, 0x0122, 0x00E2,
		0x006D, 0x0115, 0x002E, 0x0020, 0x2318, 0x0000
	};
	
	int s_len = MultiByteToWideChar(CP_UTF8, 0, s_utf8_test, -1, NULL, 0);
	if (s_len != (sizeof(s_utf16_test)/sizeof(s_utf16_test[0])))
	{
		// Required length is incorrect.
		// This usually means it's 0, in which case
		// the OS doesn't support UTF-8.
		return -1;
	}
	
	// Convert the test string from UTF-8 to UTF-16.
	wchar_t s_utf16_result[sizeof(s_utf16_test)/sizeof(s_utf16_test[0])];
	s_utf16_result[0] = 0x00;
	MultiByteToWideChar(CP_UTF8, 0, s_utf8_test, -1, s_utf16_result, sizeof(s_utf16_result)/sizeof(s_utf16_result[0]));
	
	// Verify that the string matches.
	if (wcsncmp(s_utf16_test, s_utf16_result, sizeof(s_utf16_test)/sizeof(s_utf16_test[0])) != 0)
	{
		// String doesn't match.
		return -2;
	}
	
	// UTF-8 is supported.
	return 0;
}


int WINAPI w32u_init(void)
{
	// Initialize the Win32 Unicode Translation Layer.
	if (init_counter++ != 0)
	{
		// The Win32 Unicode Translation Layer is already initialized.
		return ERR_W32U_SUCCESS;
	}
	
	// Check for UTF-8 compatibility.
	if (w32u_check_UTF8() != 0)
	{
		// System doesn't support UTF-8.
		return -ERR_W32U_UTF8_NOT_SUPPORTED;
	}
	
	// Check if the system supports Unicode.
	if (GetModuleHandleW(NULL) != NULL)
	{
		// GetModuleHandleW() returned gens.exe's module handle.
		// This means the system supports Unicode.
		
		// Check if ANSI mode is forced on the command line.
		const char *lpCmdLine = GetCommandLineA();
		if (!strstr(lpCmdLine, " --ansi"))
		{
			// ANSI mode is not forced. Enable Unicode.
			w32u_is_unicode = 1;
		}
		else
		{
			// ANSI mode is forced. Disable Unicode.
			w32u_is_unicode = 0;
		}
	}
	else
	{
		// GetModuleHandleW(NULL) returned NULL.
		// This means the system doesn't support Unicode.
		w32u_is_unicode = 0;
	}
	
	// Get DLL version numbers.
	comctl32_dll_version = GetDllVersionNumber("comctl32.dll");
	shell32_dll_version = GetDllVersionNumber("shell32.dll");
	
	// Initialize the Unicode modules.
	w32u_windows_init();
	w32u_windowsx_init();
	w32u_commctrl_init();
	w32u_shellapi_init();
	w32u_libc_init();
	w32u_commdlg_init();
	w32u_shlobj_init();
	w32u_winnls_init();
	
	// Win32 Unicode Translation Layer initialized successfully.
	return ERR_W32U_SUCCESS;
}


int WINAPI w32u_end(void)
{
	if (init_counter <= 0)
		return 0;
	
	init_counter--;
	if (init_counter > 0)
		return 0;
	
	// Disable Unicode.
	w32u_is_unicode = 0;
	
	// Clear the DLL versions.
	shell32_dll_version = 0;
	
	// Shut down components that need manual shutdowns.
	w32u_winnls_end();
	
	// TODO: Should function pointers be NULL'd?
	return 0;
}
