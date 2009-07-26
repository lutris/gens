/***************************************************************************
 * Gens: MDP: Mega Drive Plugin - Host Services. (Win32 Functions)         *
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

#include "mdp_host_gens_win32.h"

// Win32 includes.
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

// g_main_win32.hpp has winVersion, which has the OS version.
#include "emulator/g_main_win32.hpp"

// Win32 resources.
#include "ui/win32/resource.h"

// MinGW's WinAPI headers (and probably the official MS SDK)
// only define ICON_SMALL2 if _WIN32_WINNT >= 0x0501.
#ifndef ICON_SMALL2
#define ICON_SMALL2 2
#endif


/**
 * mdp_host_win32_check_icon(): Check if a window has an icon. If it doesn't, assign one.
 * @param window Window to check.
 */
void mdp_host_win32_check_icon(void *window)
{
	HWND hWnd = (HWND)window;
	HICON icon;
	
	// Check if a window has any icons.
	// A window has an icon if at least one of the following icons are set:
	// - Class Icon (Big)
	// - Class Icon (Small)
	// - Window Icon (Big)
	// - Window Icon (Small)
	// - Window Icon (Small, App Provided) (XP only)
	
	// Class Icon. (Big)
	icon = (HICON)GetClassLong(hWnd, GCL_HICON);
	if (icon)
		return;
	
	// Class Icon. (Small)
	icon = (HICON)GetClassLong(hWnd, GCL_HICONSM);
	if (icon)
		return;
	
	// Window Icon. (Big)
	icon = (HICON)SendMessage(hWnd, WM_GETICON, ICON_BIG, 0);
	if (icon)
		return;
	
	// Window Icon. (Small)
	icon = (HICON)SendMessage(hWnd, WM_GETICON, ICON_SMALL, 0);
	if (icon)
		return;
	
	// Window Icon. (Small, App Provided) (XP only)
	if (winVersion.dwMajorVersion > 5 ||
	    (winVersion.dwMajorVersion == 5 && winVersion.dwMinorVersion >= 1))
	{
		// Windows XP. Check for this icon.
		icon = (HICON)SendMessage(hWnd, WM_GETICON, ICON_SMALL2, 0);
		if (icon)
			return;
	}
	
	// Window has no icon. Set it to the Gens/GS icon.
	// TODO: Use an icon that indicates it's from a plugin instead of using IDI_GENS_APP.
	SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_GENS_APP)));
}
