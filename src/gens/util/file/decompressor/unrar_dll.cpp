/***************************************************************************
 * Gens: UnRAR.dll Management Class.                                       *
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

#include "unrar_dll.hpp"

#define InitFuncPtr_unrar(hDll, fn) p##fn = (typeof(p##fn))GetProcAddress((hDll), #fn)
#include <stdio.h>

UnRAR_dll::UnRAR_dll(const char *base_path, const char *filename)
{
	m_isLoaded = false;
	
	if (!filename)
		return;
	
	if (base_path)
		pSetCurrentDirectoryU(base_path);
	
	// TODO: Unicode translations for LoadLibrary().
	hUnrarDll = LoadLibrary(filename);
	if (!hUnrarDll)
		return;
	
	// Load the function pointers.
	InitFuncPtr_unrar(hUnrarDll, RAROpenArchiveEx);
	InitFuncPtr_unrar(hUnrarDll, RARCloseArchive);
	InitFuncPtr_unrar(hUnrarDll, RARReadHeaderEx);
	InitFuncPtr_unrar(hUnrarDll, RARProcessFile);
	InitFuncPtr_unrar(hUnrarDll, RARSetCallback);
	InitFuncPtr_unrar(hUnrarDll, RARGetDllVersion);
	
	// Check if any of the function pointers are NULL.
	if (!pRAROpenArchiveEx || !pRARCloseArchive ||
	    !pRARReadHeaderEx  || !pRARProcessFile ||
	    !pRARSetCallback   || !pRARGetDllVersion)
	{
		// NULL pointers found. That's bad.
		unload();
	}
	
	// UnRAR.dll loaded successfully.
	m_isLoaded = true;
}


UnRAR_dll::~UnRAR_dll()
{
	if (!m_isLoaded)
		return;
	
	m_isLoaded = false;
	unload();
}


void UnRAR_dll::unload(void)
{
	// Unload the DLL.
	FreeLibrary(hUnrarDll);
	hUnrarDll = NULL;
	
	// Clear the function pointers.
	pRAROpenArchiveEx	= NULL;
	pRARCloseArchive	= NULL;
	pRARReadHeaderEx	= NULL;
	pRARProcessFile		= NULL;
	pRARSetCallback		= NULL;
	pRARGetDllVersion	= NULL;
}
