/***************************************************************************
 * libgsft_w32u: Win32 Unicode Translation Layer.                          *
 * w32u.h: Main Unicode translation code.                                  *
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

#ifndef GSFT_W32U_H
#define GSFT_W32U_H

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#define MAKE_EXTFUNCPTR(f) extern typeof(f) * p##f
#define MAKE_EXTFUNCPTR2(f1, f2) extern typeof(f1) * p##f2
#define MAKE_FUNCPTR(f) typeof(f) * p##f = NULL
#define MAKE_FUNCPTR2(f1, f2) typeof(f1) * p##f2 = NULL

#ifdef __cplusplus
extern "C" {
#endif

extern BOOL w32u_is_unicode;

/** DLL versions. (0xMMNNRRRR) **/
extern DWORD shell32_dll_version;

DWORD WINAPI GetDllVersionNumber(const char *filename);

int WINAPI w32u_init(void);
int WINAPI w32u_end(void);

#ifdef __cplusplus
}
#endif

#endif /* GSFT_W32U_H */
