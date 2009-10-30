/***************************************************************************
 * libgsft: Common functions.                                              *
 * gsft_png.h: PNG handling functions.                                     *
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

#ifndef __GSFT_PNG_H
#define __GSFT_PNG_H

#include <stdlib.h>
#include <png.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Function pointers. **/
/** If GENS_PNG_DLOPEN is set, THIS MUST be initialized before using gsft_png_user_read_data()! **/
#if defined(GENS_PNG_INTERNAL) || !defined(GENS_PNG_DLOPEN)
#define gsft_png_get_io_ptr(x) png_get_io_ptr(x)
#else
#error Compiling libgsft_png.la with GENS_PNG_DLOPEN is currently broken!
#define MAKE_EXTFUNCPTR(f) extern typeof(f) * p##f
MAKE_EXTFUNCPTR(png_get_io_ptr);
#endif

/**
 * gsft_png_mem_t: PNG read from memory struct.
 */
typedef struct _gsft_png_mem_t
{
	const unsigned char *data;
	png_size_t length;
	
	/* Used internally. DO NOT MODIFY! */
	png_size_t pos;
} gsft_png_mem_t;

void gsft_png_user_read_data(png_structp png_ptr, png_bytep data, png_size_t length);

#ifdef __cplusplus
}
#endif

#endif /* __GSFT_PNG_H */
