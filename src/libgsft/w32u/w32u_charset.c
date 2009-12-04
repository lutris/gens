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

#include "w32u_charset.h"

// C includes.
#include <stdlib.h>
#include <wchar.h>

// Win32 includes.
#include <winnls.h>


/**
 * w32u_mbstowcs_alloc(): Convert a multibyte string to UTF-16 (wchar_t*).
 * @param lpSrc Source string.
 * @param cchMinSize Minimum size (in characters) for the allocated string.
 * @param cpFrom Code page to convert from.
 * @return UTF-16 (wchar_t*) string. (MUST BE free()'d AFTER USE!)
 */
wchar_t* WINAPI w32u_mbstowcs_alloc(const char *lpSrc, int cchMinSize, UINT cpFrom)
{
	int cchWcs = MultiByteToWideChar(cpFrom, 0, lpSrc, -1, NULL, 0);
	if (cchWcs <= 0)
		return NULL;
	if (cchWcs < cchMinSize)
		cchWcs = cchMinSize;
	
	wchar_t *wcs = (wchar_t*)malloc(cchWcs * sizeof(wchar_t));
	MultiByteToWideChar(CP_UTF8, 0, lpSrc, -1, wcs, cchWcs);
	return wcs;
}


/**
 * w32u_mbstombs_alloc(): Convert a multibyte string to a different multibyte encoding.
 * @param lpSrc Source string.
 * @param cbMinSize Minimum size (in bytes) for the allocated string.
 * @param cpFrom Code page to convert from.
 * @param cpTo Code page to convert to.
 * @return Multibyte string. (MUST BE free()'d AFTER USE!)
 */
char* WINAPI w32u_mbstombs_alloc(const char *lpSrc, int cbMinSize, UINT cpFrom, UINT cpTo)
{
	// Convert from cpFrom to UTF-16 first.
	int cchWcs = MultiByteToWideChar(cpFrom, 0, lpSrc, -1, NULL, 0);
	if (cchWcs <= 0)
		return NULL;
	
	wchar_t *wcs = (wchar_t*)malloc(cchWcs * sizeof(wchar_t));
	MultiByteToWideChar(cpFrom, 0, lpSrc, -1, wcs, cchWcs);
	
	// Convert from UTF-16 to cpTo.
	int cbAcs = WideCharToMultiByte(cpTo, 0, wcs, cchWcs, NULL, 0, NULL, NULL);
	if (cbAcs <= 0)
	{
		free(wcs);
		return NULL;
	}
	if (cbAcs < cbMinSize)
		cbAcs = cbMinSize;
	
	char *acs = (char*)malloc(cbAcs);
	WideCharToMultiByte(cpTo, 0, wcs, cchWcs, acs, cbAcs, NULL, NULL);
	free(wcs);
	return acs;
}


/**
 * w32u_mbstombs_copy(): Convert a multibyte string to a different multibyte encoding.
 * @param lpDest Destination buffer.
 * @param lpSrc Source buffer.
 * @param cbDest Size of the destination buffer, in bytes.
 * @param cpFrom Code page to convert from.
 * @param cpTo Code page to convert to.
 * @return 0 on success; non-zero on error.
 */
int WINAPI w32u_mbstombs_copy(char *lpDest, const char *lpSrc, int cbDest, UINT cpFrom, UINT cpTo)
{
	// Convert from cpFromx to UTF-16 first.
	int cchWcs = MultiByteToWideChar(cpFrom, 0, lpSrc, -1, NULL, 0);
	if (cchWcs <= 0)
		return -1;
	
	wchar_t *wcs = (wchar_t*)malloc(cchWcs * sizeof(wchar_t));
	MultiByteToWideChar(cpFrom, 0, lpSrc, -1, wcs, cchWcs);
	
	// Convert from UTF-16 to cpTo.
	int cbAcs = WideCharToMultiByte(cpTo, 0, wcs, cchWcs, NULL, 0, NULL, NULL);
	if (cbAcs <= 0 || cbAcs > cbDest)
	{
		free(wcs);
		return -2;
	}
	
	WideCharToMultiByte(cpTo, 0, wcs, cchWcs, lpDest, cbAcs, NULL, NULL);
	free(wcs);
	return 0;
}


/**
 * w32u_mbstombs_alloc(): Convert a multibyte string to a different multibyte encoding.
 * @param lpSrc String buffer to convert. (Must be null-terminated!)
 * @param cbBuf Size of the string buffer.
 * @param cpFrom Code page to convert from.
 * @param cpTo Code page to convert to.
 * @return 0 on success; non-zero on error.
 */
int WINAPI w32u_mbstombs_ip(char *lpBuf, int cbBuf, UINT cpFrom, UINT cpTo)
{
	// Convert from cpFrom to UTF-16 first.
	int cchWcs = MultiByteToWideChar(cpFrom, 0, lpBuf, -1, NULL, 0);
	if (cchWcs <= 0)
		return -1;
	
	wchar_t *wcs = (wchar_t*)malloc(cchWcs * sizeof(wchar_t));
	MultiByteToWideChar(cpFrom, 0, lpBuf, -1, wcs, cchWcs);
	
	// Convert from UTF-16 to cpTo.
	int cbAcs = WideCharToMultiByte(cpTo, 0, wcs, cchWcs, NULL, 0, NULL, NULL);
	if (cbAcs <= 0 || cbAcs > cbBuf)
	{
		free(wcs);
		return -2;
	}
	
	WideCharToMultiByte(cpTo, 0, wcs, cchWcs, lpBuf, cbAcs, NULL, NULL);
	free(wcs);
	return 0;
}
