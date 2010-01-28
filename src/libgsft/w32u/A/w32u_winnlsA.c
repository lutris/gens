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

// Handle to kernel32.dll.
static HMODULE hKernel32 = NULL;

// GetCPInfoExA() is not available on Windows 95 or Windows NT 4.0.
MAKE_STFUNCPTR(GetCPInfoExA);


static WINBASEAPI BOOL WINAPI GetCPInfoExUA(UINT CodePage, DWORD dwFlags, LPCPINFOEXA lpCPInfoEx)
{
	if (!lpCPInfoEx)
	{
		// Struct not specified. Don't bother converting anything.
		return pGetCPInfoExA(CodePage, dwFlags, lpCPInfoEx);
	}
	
	// Get the code page information.
	BOOL bRet = pGetCPInfoExA(CodePage, dwFlags, lpCPInfoEx);
	
	// Convert the code page name from ANSI to UTF-8.
	w32u_ANSItoUTF8_ip(lpCPInfoEx->CodePageName, sizeof(lpCPInfoEx->CodePageName));
	lpCPInfoEx->CodePageName[sizeof(lpCPInfoEx->CodePageName)-1] = 0x00;
	return bRet;
}


static WINBASEAPI BOOL WINAPI GetCPInfoExUA_EMULATE(UINT CodePage, DWORD dwFlags, LPCPINFOEXA lpCPInfoEx)
{
	// Emulate GetCPInfoExUA() on platforms that don't have GetCPInfoExA().
	// This can be done by using GetCPInfo(), then filling in the other fields.
	if (!lpCPInfoEx)
	{
		// Struct not specified. Don't bother converting anything.
		return GetCPInfo(CodePage, (LPCPINFO)lpCPInfoEx);
	}
	
	// Get the code page information.
	BOOL bRet = GetCPInfo(CodePage, (LPCPINFO)lpCPInfoEx);
	if (!bRet)
		return bRet;
	
	// Get the other fields for the CPINFOEX struct.
	switch (CodePage)
	{
		case CP_ACP:
			lpCPInfoEx->CodePage = GetACP();
			break;
		case CP_OEMCP:
			lpCPInfoEx->CodePage = GetOEMCP();
			break;
		default:
			lpCPInfoEx->CodePage = CodePage;
			break;
	}
	
	// Use a reasonable default for the Unicode default character.
	lpCPInfoEx->UnicodeDefaultChar = L'?';
	
	// Clear the code page name for now.
	// TODO: Add a lookup table.
	lpCPInfoEx->CodePageName[0] = 0x00;
	
	return bRet;
}


void WINAPI w32u_winnlsA_init(void)
{
	// Load kernel32.dll.
	hKernel32 = LoadLibraryA("kernel32.dll");
	if (!hKernel32)
		return;
	
	pGetCPInfoExA = (typeof(pGetCPInfoExA))GetProcAddress(hKernel32, "GetCPInfoExA");
	if (pGetCPInfoExA)
		pGetCPInfoExU = &GetCPInfoExUA;
	else
		pGetCPInfoExU = &GetCPInfoExUA_EMULATE;
}


void WINAPI w32u_winnlsA_end(void)
{
	// NULL out the kernel32.dll function pointers.
	pGetCPInfoExA = NULL;
	
	// Unload kernel32.dll.
	if (hKernel32)
		FreeLibrary(hKernel32);
	
	hKernel32 = NULL;
}
