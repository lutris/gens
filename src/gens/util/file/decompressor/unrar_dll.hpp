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

#ifndef GENS_DECOMPRESSOR_UNRAR_DLL_HPP
#define GENS_DECOMPRESSOR_UNRAR_DLL_HPP

#ifndef _WIN32
#error unrar_dll.cpp only works on Win32.
#endif

#include "libgsft/w32u/w32u_windows.h"
#include "unrar.h"

#ifdef __cplusplus

#ifndef MAKE_CLASSFUNCPTR
#define MAKE_CLASSFUNCPTR(f) typeof(f) * p##f
#endif

class UnRAR_dll
{
	public:
		UnRAR_dll(void);
		~UnRAR_dll();
		
		bool load(const char *filename, const char *base_path = NULL);
		void unload(void);
		
		bool isLoaded(void) { return m_isLoaded; }
		
		MAKE_CLASSFUNCPTR(RAROpenArchiveEx);
		MAKE_CLASSFUNCPTR(RARCloseArchive);
		MAKE_CLASSFUNCPTR(RARReadHeaderEx);
		MAKE_CLASSFUNCPTR(RARProcessFile);
		MAKE_CLASSFUNCPTR(RARSetCallback);
		MAKE_CLASSFUNCPTR(RARGetDllVersion);
	
	private:
		bool m_isLoaded;
		HINSTANCE hUnrarDll;
};

#endif

#endif /* GENS_DECOMPRESSOR_UNRAR_DLL_HPP */
