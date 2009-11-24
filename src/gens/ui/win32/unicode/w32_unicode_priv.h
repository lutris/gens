/***************************************************************************
 * Gens: (Win32) Unicode Translation Layer. (Private Macros)               *
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

#ifndef GENS_W32_UNICODE_PRIV_H
#define GENS_W32_UNICODE_PRIV_H

#define MAKE_FUNCPTR(f) typeof(f) * p##f = NULL
#define MAKE_STFUNCPTR(f) static typeof(f) * p##f = NULL

/**
 * InitFuncPtrsU(): Initialize function pointers for functions that need text conversions.
 */
#define InitFuncPtrsU(hDLL, fn, pW, pA, pU) \
do { \
	pW = (typeof(pW))GetProcAddress(hDLL, fn "W"); \
	if (pW) \
		pA = &pU; \
	else \
		pA = (typeof(pA))GetProcAddress(hDLL, fn "A"); \
} while (0)

/**
 * InitFuncPtrsU(): Initialize function pointers for functions that don't need text conversions.
 */
#define InitFuncPtrs(hDLL, fn, pA) \
do { \
	pA = (typeof(pA))GetProcAddress(hDLL, fn "W"); \
	if (!pA) \
		pA = (typeof(pA))GetProcAddress(hDLL, fn "A"); \
} while (0)

#endif /* GENS_W32_UNICODE_PRIV_H */
