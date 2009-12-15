/***************************************************************************
 * libgsft_w32u: Win32 Unicode Translation Layer.                          *
 * w32u_winnlsW.c: winnls.h translation. (Unicode version)                 *
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

#include "w32u_winnlsW.h"
#include "w32u_winnls.h"
#include "w32u_charset.h"

// C includes.
#include <stdlib.h>
#include <string.h>


static WINBASEAPI BOOL WINAPI GetCPInfoExUW(UINT CodePage, DWORD dwFlags, LPCPINFOEXA lpCPInfoEx)
{
	if (!lpCPInfoEx)
	{
		// Struct not specified. Don't bother converting anything.
		return GetCPInfoExW(CodePage, dwFlags, (LPCPINFOEXW)lpCPInfoEx);
	}
	
	// Get the code page information in a temporary struct.
	CPINFOEXW wCPInfoEx;
	BOOL bRet = GetCPInfoExW(CodePage, dwFlags, &wCPInfoEx);
	
	// Copy the non-text elements to the original struct.
	memcpy(lpCPInfoEx, &wCPInfoEx, (sizeof(wCPInfoEx) - sizeof(wCPInfoEx.CodePageName)));
	
	// Convert the code page name from UTF-16 to UTF-8.
	w32u_UTF16toUTF8_copy(lpCPInfoEx->CodePageName, wCPInfoEx.CodePageName, sizeof(lpCPInfoEx->CodePageName));
	lpCPInfoEx->CodePageName[sizeof(lpCPInfoEx->CodePageName)-1] = 0x00;
	return bRet;
}


void WINAPI w32u_winnlsW_init(void)
{
	pGetCPInfoExU		= &GetCPInfoExUW;
}
