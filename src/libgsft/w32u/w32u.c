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
#include "w32u_priv.h"
#include "w32u_windows.h"
#include "w32u_shellapi.h"
#include "w32u_libc.h"
#include "w32u_commdlg.h"
#include "w32u_shlobj.h"

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

// DLLs.
static HMODULE hKernel32 = NULL;
static HMODULE hUser32 = NULL;
static HMODULE hShell32 = NULL;

/** DLL versions. (0xMMNNRRRR) **/
DWORD shell32_dll_version = 0;


/**
 * GetDllVersionNumber(): Get a DLL's version number via DllGetVersion().
 * @param hDLL Handle to DLL.
 * @return DLL version number, or 0 on error.
 */
DWORD WINAPI GetDllVersionNumber(HMODULE hDll)
{
	DLLVERSIONINFO dllvi;
	DLLGETVERSIONPROC dllviproc;
	HRESULT hRet;
	dllvi.cbSize = sizeof(dllvi);
	
	dllviproc = (DLLGETVERSIONPROC)GetProcAddress(hDll, "DllGetVersion");
	if (!dllviproc)
		return 0;
	
	// DllGetVersion() found.
	hRet = dllviproc(&dllvi);
	if (hRet != S_OK)
		return 0;
	
	// Received the version number.
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
	
	// Enable Unicode on Windows NT only.
	// TODO: MSLU support.
	OSVERSIONINFOA osv;
	memset(&osv, 0x00, sizeof(osv));
	osv.dwOSVersionInfoSize = sizeof(osv);
	GetVersionExA(&osv);
	
#ifndef W32U_NO_UNICODE
	if (osv.dwPlatformId == VER_PLATFORM_WIN32_NT)
	{
		// Windows NT. Enable Unicode.
		w32u_is_unicode = 1;
	}
	else
#endif
	{
		// Windows 9x. Disable Unicode.
		// TODO: MSLU support.
		w32u_is_unicode = 0;
	}
	
	// Load the DLLs.
	// TODO: Error handling.
	hKernel32 = LoadLibrary("kernel32.dll");
	hUser32 = LoadLibrary("user32.dll");
	hShell32 = LoadLibrary("shell32.dll");
	
	// Initialize windows.h
	w32u_windows_init(hKernel32, hUser32);
	
	// Get DLL version numbers.
	shell32_dll_version = GetDllVersionNumber(hShell32);
	
	// Other Win32 Unicode modules.
	w32u_shellapi_init(hShell32);
	w32u_libc_init();
	w32u_commdlg_init();
	w32u_shlobj_init(hShell32);
	
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
	
	// Unload the libraries.
	FreeLibrary(hKernel32);
	hKernel32 = NULL;
	FreeLibrary(hUser32);
	hUser32 = NULL;
	FreeLibrary(hShell32);
	hShell32 = NULL;
	
	// Clear the DLL versions.
	shell32_dll_version = 0;
	
	// TODO: Should function pointers be NULL'd?
	// TODO: Should shellapi and shlobj have end functions?
	//w32u_shellapi_end();
	w32u_libc_end();
	w32u_commdlg_end();
	//w32u_shlobj_end();
	
	return 0;
}
