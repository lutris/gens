/***************************************************************************
 * libgsft_w32u: Win32 Unicode Translation Layer.                          *
 * w32u_priv.h: Private functions.                                         *
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

#ifndef GSFT_W32U_PRIV_H
#define GSFT_W32U_PRIV_H

#include "w32u.h"

#define MAKE_FUNCPTR(f) typeof(f) * p##f = NULL
#define MAKE_FUNCPTR2(f1, f2) typeof(f1) * p##f2 = NULL
#define MAKE_STFUNCPTR(f) static typeof(f) * p##f = NULL

#define InitFuncPtrUW(hDll, fnU, fnW) \
do { \
	p##fnW = (typeof(p##fnW))GetProcAddress(hDll, #fnW); \
	p##fnU = &(fnU##W); \
} while (0)

#define InitFuncPtrUA(hDll, fnU, fnA) \
do { \
	p##fnA = (typeof(p##fnA))GetProcAddress(hDll, #fnA); \
	p##fnU = &(fnU##A); \
} while (0)

#define InitFuncPtrU(hDll, fnU, fn) \
do { \
	p##fnU = (typeof(p##fnU))GetProcAddress(hDll, #fn); \
} while (0)

/**
 * InitFuncPtr(): Initialize a function pointer.
 */
#define InitFuncPtr(hDll, fn) \
do { \
	p##fn = (typeof(p##fn))GetProcAddress(hDll, #fn); \
} while (0)

/**
 * InitFuncPtrsU(): Initialize function pointers for functions that need text conversions.
 */
#define InitFuncPtrsU(hDll, fn, pW, pA, pU) \
do { \
	if (w32u_is_unicode) \
	{ \
		pW = (typeof(pW))GetProcAddress(hDll, fn "W"); \
		pA = &pU; \
	} \
	else \
		pA = (typeof(pA))GetProcAddress(hDll, fn "A"); \
} while (0)

/**
 * InitFuncPtrs(): Initialize function pointers for functions that don't need text conversions.
 */
#define InitFuncPtrs(hDll, fn, pA) \
do { \
	if (w32u_is_unicode) \
		pA = (typeof(pA))GetProcAddress(hDll, fn "W"); \
	else \
		pA = (typeof(pA))GetProcAddress(hDll, fn "A"); \
} while (0)

/**
 * InitFuncPtrsU_libc(): Initialize function pointers for functions that need text conversions. (libc version)
 */
#define InitFuncPtrsU_libc(hDll, fnA, fnW, pW, pA, pU) \
do { \
	if (w32u_is_unicode) \
	{ \
		pW = (typeof(pW))GetProcAddress(hDll, fnW); \
		pA = &pU; \
	} \
	else \
		pA = (typeof(pA))GetProcAddress(hDll, fnA); \
} while (0)

#ifdef __cplusplus
extern "C" {
#endif

/** Functions. **/
wchar_t* WINAPI w32u_mbstowcs(const char *mbs);

#ifdef __cplusplus
}
#endif

#endif /* GSFT_W32U_PRIV_H */
