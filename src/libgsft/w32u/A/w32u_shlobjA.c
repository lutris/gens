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

#include "w32u_shlobjA.h"
#include "w32u_shlobj.h"
#include "w32u_charset.h"

// C includes.
#include <stdlib.h>


static LPITEMIDLIST WINAPI SHBrowseForFolderUA(PBROWSEINFO lpbi)
{
	// BROWSEINFOA and BROWSEINFOW don't contain any actual string data,
	// so the structs can be copied.
	BROWSEINFOA abi;
	memcpy(&abi, lpbi, sizeof(abi));
	
	// Convert constant strings from UTF-8 to ANSI.
	char *lpszaTitle = NULL;
	
	if (lpbi->lpszTitle)
	{
		lpszaTitle = w32u_UTF8toANSI(lpbi->lpszTitle);
		abi.lpszTitle = lpszaTitle;
	}
	
	// Allocate the return buffer.
	char *pszaDisplayName = NULL;
	if (lpbi->pszDisplayName)
	{
		// This is assumed to be MAX_PATH characters.
		pszaDisplayName = (char*)malloc(MAX_PATH * sizeof(char));
		abi.pszDisplayName = pszaDisplayName;
	}
	
	// Get the directory list entry.
	LPITEMIDLIST pidl = SHBrowseForFolderA(&abi);
	
	// Convert the non-constant strings from ANSI to UTF-8.
	if (abi.pszDisplayName)
	{
		w32u_ANSItoUTF8_copy(lpbi->pszDisplayName, abi.pszDisplayName, MAX_PATH);
	}
	
	// Free the strings.
	free(lpszaTitle);
	free(pszaDisplayName);
	return pidl;
}


static BOOL WINAPI SHGetPathFromIDListUA(LPCITEMIDLIST pidl, LPSTR pszPath)
{
	if (!pszPath)
	{
		// No return buffer was specified.
		return SHGetPathFromIDListA(pidl, pszPath);
	}
	
	// Allocate an ANSI character buffer.
	// Size is assumed to be MAX_PATH characters.
	char *pszaPath = (char*)malloc(MAX_PATH * sizeof(char));
	
	// Get the directory.
	BOOL bRet = SHGetPathFromIDListA(pidl, pszaPath);
	
	// Convert the text from ANSI to UTF-8.
	w32u_ANSItoUTF8_copy(pszPath, pszaPath, MAX_PATH);
	
	// Free the buffer.
	free(pszaPath);
	return bRet;
}


void WINAPI w32u_shlobjA_init(void)
{
	pSHBrowseForFolderU	= &SHBrowseForFolderUA;
	pSHGetPathFromIDListU	= &SHGetPathFromIDListUA;
}
