/***************************************************************************
 * libgsft_w32u: Win32 Unicode Translation Layer.                          *
 * w32u_winnlsA.c: winnls.h translation. (ANSI version)                    *
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

#include "w32u_winnlsA.h"
#include "w32u_winnls.h"
#include "w32u_charset.h"

// C includes.
#include <stdlib.h>
#include <string.h>


static WINBASEAPI BOOL WINAPI GetCPInfoExUA(UINT CodePage, DWORD dwFlags, LPCPINFOEXA lpCPInfoEx)
{
	if (!lpCPInfoEx)
	{
		// Struct not specified. Don't bother converting anything.
		return GetCPInfoExA(CodePage, dwFlags, lpCPInfoEx);
	}
	
	// Get the code page information.
	BOOL bRet = GetCPInfoExA(CodePage, dwFlags, lpCPInfoEx);
	
	// Convert the code page name from ANSI to UTF-8.
	printf("cpa: %s\n", lpCPInfoEx->CodePageName);
	w32u_ANSItoUTF8_ip(lpCPInfoEx->CodePageName, sizeof(lpCPInfoEx->CodePageName));
	printf("cpu: %s\n", lpCPInfoEx->CodePageName);
	lpCPInfoEx->CodePageName[sizeof(lpCPInfoEx->CodePageName)-1] = 0x00;
	return bRet;
}


void WINAPI w32u_winnlsA_init(void)
{
	pGetCPInfoExU		= &GetCPInfoExUA;
}
