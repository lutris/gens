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


int WINAPI w32u_init(void)
{
	// Initialize the Win32 Unicode Translation Layer.
	if (init_counter++ != 0)
	{
		// The Win32 Unicode Translation Layer is already initialized.
		return 0;
	}
	
	// Check if the system supports Unicode.
#ifndef W32U_NO_UNICODE
	if (GetModuleHandleW(NULL) != NULL)
	{
		// GetModuleHandleW() returned gens.exe's module handle.
		// This means the system supports Unicode.
		w32u_is_unicode = 1;
	}
	else
#endif
	{
		// GetModuleHandleW(NULL) returned NULL.
		// This means the system doesn't support Unicode.
		w32u_is_unicode = 0;
	}
	
	// Get DLL version numbers.
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
	return 0;
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
	
	// TODO: Should function pointers be NULL'd?
	return 0;
}
