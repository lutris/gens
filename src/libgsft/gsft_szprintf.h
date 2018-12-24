/***************************************************************************
 * libgsft: Common functions.                                              *
 * gsft_szprintf.h: snprintf() functions with automatic null termination.  *
 *                                                                         *
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

#ifndef __GSFT_SZPRINTF_H
#define __GSFT_SZPRINTF_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

// TODO: Add wrappers for _sntprintf() [Win32 TCHAR].


/**
 * szprintf(): snprintf() wrapper with automatic NULL termination.
 */
#define szprintf(str, size, format, ...) \
do { \
	snprintf(str, size, format, ##__VA_ARGS__); \
	str[size-1] = 0x00; \
} while (0)


/**
 * vszprintf(): vsnprintf() wrapper with automatic NULL termination.
 */
#define vszprintf(str, size, format, ap) \
do { \
	vsnprintf(str, size, format, ap); \
	str[size-1] = 0x00; \
} while (0)


#endif /* __GSFT_SZPRINTF_H */
