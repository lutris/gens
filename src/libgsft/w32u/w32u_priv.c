/***************************************************************************
 * libgsft_w32u: Win32 Unicode Translation Layer.                          *
 * w32u_priv.c: Private functions.                                         *
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

#include "w32u_priv.h"

// C includes.
#include <stdlib.h>
#include <wchar.h>

// Win32 includes.
#include <winnls.h>


/**
 * w32u_UTF8toUTF16(): Convert a UTF-8 string to UTF-16 (wchar_t*).
 * @param mbs UTF-8 string.
 * @return UTF-16 (wchar_t*) string. (MUST BE free()'d AFTER USE!)
 */
wchar_t* WINAPI w32u_UTF8toUTF16(const char *mbs)
{
	int cchWcs = MultiByteToWideChar(CP_UTF8, 0, mbs, -1, NULL, 0);
	if (cchWcs <= 0)
		return NULL;
	
	wchar_t *wcs = (wchar_t*)malloc(cchWcs * sizeof(wchar_t));
	MultiByteToWideChar(CP_UTF8, 0, mbs, -1, wcs, cchWcs);
	return wcs;
}


/**
 * w32u_UTF8toUTF16(): Convert a UTF-8 string to the local ANSI code page.
 * @param mbs UTF-8 string.
 * @return ANSI string. (MUST BE free()'d AFTER USE!)
 */
char* WINAPI w32u_UTF8toANSI(const char *mbs)
{
	// Convert from UTF-8 to UTF-16 first.
	int cchWcs = MultiByteToWideChar(CP_UTF8, 0, mbs, -1, NULL, 0);
	if (cchWcs <= 0)
		return NULL;
	
	wchar_t *wcs = (wchar_t*)malloc(cchWcs * sizeof(wchar_t));
	MultiByteToWideChar(CP_UTF8, 0, mbs, -1, wcs, cchWcs);
	
	// Convert from UTF-16 to ANSI.
	// TODO: Use GetCPInfo() to get the system default replacement character.
	int cbAcs = WideCharToMultiByte(CP_ACP, 0, wcs, -1, NULL, 0, NULL, NULL);
	if (cbAcs <= 0)
	{
		free(wcs);
		return NULL;
	}
	
	char *acs = (char*)malloc(cbAcs);
	WideCharToMultiByte(CP_ACP, 0, wcs, -1, acs, cbAcs, NULL, NULL);
	free(wcs);
	return acs;
}
