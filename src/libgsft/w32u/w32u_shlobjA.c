/***************************************************************************
 * libgsft_w32u: Win32 Unicode Translation Layer.                          *
 * w32u_shlobjA.c: shlobj.h translation. (ANSI version)                    *
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
#include "w32u_shlobj.h"

// C includes.
#include <stdlib.h>


static LPITEMIDLIST WINAPI SHBrowseForFolderUA(PBROWSEINFO lpbi)
{
	// TODO: ANSI conversion.
	return SHBrowseForFolderA(lpbi);
}


static BOOL WINAPI SHGetPathFromIDListUA(LPCITEMIDLIST pidl, LPSTR pszPath)
{
	// TODO: ANSI conversion.
	return SHGetPathFromIDListA(pidl, pszPath);
}


int WINAPI w32u_shlobjA_init(void)
{
	// TODO: Error handling.
	
	pSHBrowseForFolderU	= &SHBrowseForFolderUA;
	pSHGetPathFromIDListU	= &SHGetPathFromIDListUA;
	
	return 0;
}
