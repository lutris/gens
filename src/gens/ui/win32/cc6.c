/***************************************************************************
 * Gens: (Win32) Common Controls 6.                                        *
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

#include "cc6.h"

// C includes.
#include <string.h>

// Win32 includes.
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <tchar.h>
#include <shlwapi.h>


/**
 * cc6_check(): Check for Common Controls v6.0.
 * @return Common Controls v6.0 status.
 */
CC6_STATUS_T cc6_check(void)
{
	// TODO: Check the version of COMCTL32.DLL currently loaded in memory.
	// If the program's manifest isn't set up properly, then this will return
	// CC6_STATUS_V6 on Windows XP or later, but v6.0 won't actually be loaded!
	
	static const TCHAR dllFileName[] = TEXT("\\COMCTL32.DLL");
	
	TCHAR dllPath[MAX_PATH];
	unsigned int rval;
	
	// COMCTL32.DLL is located in %SYSDIR%\\COMCTL32.DLL.
	// Get the system directory.
	rval = GetSystemDirectory(dllPath, (sizeof(dllPath) / sizeof(dllPath[0])));
	if (rval == 0 || rval > (sizeof(dllPath) - sizeof(dllFileName) - 1))
	{
		// Could not retrieve the system directory.
		// Assume we're using an older version of comctl32.dll.
		return CC6_STATUS_OLD;
	}
	
	// Append the filename.
	// NOTE: _tcscat() [TCHAR strcat()] can be used here, since the amonut of space
	// remaining in the buffer was checked via GetSystemDirectory()'s return value.
	_tcscat(dllPath, dllFileName);
	
	// Load the DLL.
	HINSTANCE hinstDLL = LoadLibrary(dllPath);
	if (!hinstDLL)
	{
		// Could not load the DLL.
		// Assume we're using an older version of comctl32.dll.
		return CC6_STATUS_OLD;
	}
	
	// Get the DllGetVersion() function.
	// References:
	// - Shell and Common Controls Versions: http://msdn.microsoft.com/en-us/library/bb776779%28VS.85%29.aspx
	// - DllGetVersion Function (): http://msdn.microsoft.com/en-us/library/bb776404%28VS.85%29.aspx
	DLLGETVERSIONPROC pDllGetVersion =
			(DLLGETVERSIONPROC)GetProcAddress(hinstDLL, "DllGetVersion");
	if (!pDllGetVersion)
	{
		// Could not find the DllGetVersion symbol.
		// Assume we're using an older version of comctl32.dll.
		FreeLibrary(hinstDLL);
		return CC6_STATUS_OLD;
	}
	
	// Get the DLL version.
	DLLVERSIONINFO dvi;
	HRESULT hr;
	
	memset(&dvi, 0x00, sizeof(dvi));
	dvi.cbSize = sizeof(dvi);
	
	hr = pDllGetVersion(&dvi);
	FreeLibrary(hinstDLL);
	
	if (FAILED(hr))
	{
		// An error occurred calling DllGetVersion().
		// Assume we're using an older version of comctl32.dll.
		return CC6_STATUS_OLD;
	}
	
	// Check the DLL version number.
	if (dvi.dwMajorVersion > 5 ||
	    (dvi.dwMajorVersion == 5 && dvi.dwMinorVersion >= 82))
	{
		// Common Controls v6.0 (DLL version 5.82)
		return CC6_STATUS_V6;
	}
	
	// This is an older version of Common Controls.
	return CC6_STATUS_OLD;
}
