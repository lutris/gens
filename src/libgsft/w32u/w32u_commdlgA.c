/***************************************************************************
 * libgsft_w32u: Win32 Unicode Translation Layer.                          *
 * w32u_commdlgA.c: commdlg.h translation. (ANSI version)                  *
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
#include "w32u_commdlgW.h"

#include "w32u_commdlg.h"

// C includes.
#include <stdlib.h>
#include <string.h>


typedef BOOL (WINAPI *GETOPENFILENAME_PROC)(void *lpofn);
static inline BOOL WINAPI GetOpenFileNameUA_int(LPOPENFILENAMEA lpofn, GETOPENFILENAME_PROC fn)
{
	// TODO: ANSI conversion.
	return fn(lpofn);
}


static BOOL WINAPI GetOpenFileNameUA(LPOPENFILENAMEA lpofn)
{
	return GetOpenFileNameUA_int(lpofn, &GetOpenFileNameA);
}


static BOOL WINAPI GetSaveFileNameUA(LPOPENFILENAMEA lpofn)
{
	return GetOpenFileNameUA_int(lpofn, &GetSaveFileNameA);
}


int WINAPI w32u_commdlgA_init(void)
{
	// TODO: Error handling.
	
	pGetOpenFileNameU	= &GetOpenFileNameUA;
	pGetSaveFileNameU	= &GetSaveFileNameUA;
	
	return 0;
}


int WINAPI w32u_commdlgA_end(void)
{
	// TODO: Should function pointers be NULL'd?
	return 0;
}
