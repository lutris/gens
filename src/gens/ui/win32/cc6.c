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
#include "libgsft/w32u/w32u.h"
#include "libgsft/w32u/w32u_windows.h"
//#include <shlwapi.h>

// libgsft includes.
#include "libgsft/gsft_strlcpy.h"


/**
 * cc6_check(): Check for Common Controls v6.0.
 * @return Common Controls v6.0 status.
 */
CC6_STATUS_T WINAPI cc6_check(void)
{
	// TODO: Check the version of COMCTL32.DLL currently loaded in memory.
	// If the program's manifest isn't set up properly, then this will return
	// CC6_STATUS_V6 on Windows XP or later, but v6.0 won't actually be loaded!
	
	static const char dllFileName[] = "\\COMCTL32.DLL";
	char dllPath[MAX_PATH];
	
	// COMCTL32.DLL is located in %SYSDIR%\\COMCTL32.DLL.
	// Get the system directory.
	UINT uRet = pGetSystemDirectoryU(dllPath, sizeof(dllPath));
	if (uRet == 0 || uRet > (sizeof(dllPath) - sizeof(dllFileName) - 1))
	{
		// Could not retrieve the system directory.
		// Assume we're using an older version of comctl32.dll.
		return CC6_STATUS_OLD;
	}
	
	// Append the filename.
	strlcat(dllPath, dllFileName, sizeof(dllPath));
	
	// Get the DLL version number.
	DWORD dllVersion = GetDllVersionNumber(dllFileName);
	if (dllVersion >= 0x05520000)
	{
		// Common Controls v6.0 (DLL version 5.82)
		return CC6_STATUS_V6;
	}
	
	// This is an older version of Common Controls.
	return CC6_STATUS_OLD;
}
