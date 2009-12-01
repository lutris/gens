/***************************************************************************
 * libgsft_w32u: Win32 Unicode Translation Layer.                          *
 * w32u_libcA.c: libc translation. (ANSI version)                          *
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

#include "w32u_windows.h"
#include "w32u_priv.h"
#include "w32u_libcA.h"

#include "w32u_libc.h"

// C includes.
#include <stdlib.h>

// Initialization counter.
static int init_counter = 0;

// DLLs.
static HMODULE hMsvcrt = NULL;


MAKE_STFUNCPTR2(access, accessA);
static int accessUA(const char *path, int mode)
{
	// TODO: ANSI conversion.
	return paccessA(path, mode);
}


MAKE_STFUNCPTR2(fopen, fopenA);
static FILE* fopenUA(const char *path, const char *mode)
{
	// TODO: ANSI conversion.
	return pfopenA(path, mode);
}


int WINAPI w32u_libcA_init(void)
{
	if (init_counter++ != 0)
		return 0;
	
	// TODO: Error handling.
	hMsvcrt = LoadLibrary("msvcrt.dll");
	
	paccessA = (typeof(paccessA))GetProcAddress(hMsvcrt, "_access");
	if (!paccessA)
		paccessA = (typeof(paccessA))GetProcAddress(hMsvcrt, "access");
	paccess = &accessUA;
	
	pfopenA = (typeof(pfopenA))GetProcAddress(hMsvcrt, "fopen");
	if (!pfopenA)
		pfopenA = (typeof(pfopenA))GetProcAddress(hMsvcrt, "_fopen");
	pfopen = &fopenUA;
	
	InitFuncPtr(hMsvcrt, _wcsicmp);
	
	return 0;
}


int WINAPI w32u_libcA_end(void)
{
	if (init_counter <= 0)
		return 0;
	
	init_counter--;
	if (init_counter > 0)
		return 0;
	
	FreeLibrary(hMsvcrt);
	hMsvcrt = NULL;
	
	// TODO: Should the function pointers be NULL'd?
	return 0;
}
