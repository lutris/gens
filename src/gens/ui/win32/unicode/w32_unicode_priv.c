/***************************************************************************
 * Gens: (Win32) Unicode Translation Layer. (Private Functions             *
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

#include "w32_unicode_priv.h"

// C includes.
#include <stdlib.h>
#include <wchar.h>

// Win32 includes.
#include <winnls.h>


/**
 * w32_mbstowcs(): Convert a multibyte (UTF-8) string to a wide-character (UTF-16) string.
 * @param mbs Multibyte string.
 * @return Wide-character string. (MUST BE free()'d AFTER USE!)
 */
wchar_t* WINAPI w32_mbstowcs(const char *mbs)
{
	int len = pMultiByteToWideChar(CP_UTF8, 0, mbs, -1, NULL, 0);
	if (len <= 0)
		return NULL;
	
	len *= sizeof(wchar_t);
	wchar_t *wcs = (wchar_t*)malloc(len);
	
	pMultiByteToWideChar(CP_UTF8, 0, mbs, -1, wcs, len);
	return wcs;
}
