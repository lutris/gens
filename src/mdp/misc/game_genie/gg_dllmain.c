/***************************************************************************
 * MDP: Game Genie. (Win32 DllMain() function.)                            *
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

#include "gg_dllmain.h"

// Instance.
HINSTANCE gg_hInstance = NULL;

/**
 * DllMain(): Win32 DLL startup function.
 * @param hinstDLL hInstnace of the DLL.
 * @param fdwReason
 * @param lpReserved
 * @return TRUE on success; FALSE on failure.
 */
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
	MDP_UNUSED_PARAMETER(lpReserved);
	
	if (fdwReason == DLL_PROCESS_ATTACH)
		gg_hInstance = hinstDLL;
	
	return TRUE;
}
