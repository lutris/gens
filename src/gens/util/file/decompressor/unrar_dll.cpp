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

UnRAR_dll::UnRAR_dll(void)
{
	m_isLoaded = false;
}


/**
 * UnRAR_dll::load(): Load UnRAR.dll.
 * @param base_path Base path to cd into.
 * @param filename Filename of UnRAR.dll.
 * @return true on success; false on failure.
 */
bool UnRAR_dll::load(const char *filename, const char *base_path)
{
	if (m_isLoaded)
		return true;
	
	if (!filename)
		return false;
	
	// If a base path is specified, cd to it.
	// TODO: Append the base path to filename instead?
	if (base_path)
		pSetCurrentDirectoryU(base_path);
	
	// NOTE: Only ANSI filenames will be accepted here.
	hUnrarDll = LoadLibraryA(filename);
	if (!hUnrarDll)
		return false;
	
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
	return true;
}


UnRAR_dll::~UnRAR_dll()
{
	if (!m_isLoaded)
		return;
	
	// Unload the DLL.
	unload();
}


void UnRAR_dll::unload(void)
{
	if (!m_isLoaded)
		return;
	
	// Mark the DLL as unloaded.
	m_isLoaded = false;
	
	// Clear the function pointers.
	pRAROpenArchiveEx	= NULL;
	pRARCloseArchive	= NULL;
	pRARReadHeaderEx	= NULL;
	pRARProcessFile		= NULL;
	pRARSetCallback		= NULL;
	pRARGetDllVersion	= NULL;
	
	// Unload the DLL.
	FreeLibrary(hUnrarDll);
	hUnrarDll = NULL;
}
