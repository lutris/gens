/***************************************************************************
 * Gens: Bitmap Header Struct.                                             *
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

#ifndef GENS_BMP_H
#define GENS_BMP_H

/* Make sure structs are packed. */
#ifdef PACKED
#undef PACKED
#endif

#include <stdint.h>

#define PACKED __attribute__ ((packed))

#ifdef __cplusplus
extern "C" {
#endif

/* All 16-bit and 32-bit values are stored in little-endian. */

typedef struct PACKED _bmp_header_t
{
	char magic_number[2];		/* Magic Number. {'B', 'M'} */
	uint32_t size;			/* Size of the bitmap, including the header. (bytes) */
	uint16_t reserved1;
	uint16_t reserved2;
	uint32_t bmp_start;		/* Bitmap starting position. */
	uint32_t bmp_header_size;	/* Bitmap header size, in bytes, starting from here. */
	uint32_t width;			/* Width. (pixels) */
	uint32_t height;		/* Height. (pixels) */
	uint16_t planes;		/* Number of planes. (always 1) */
	uint16_t bpp;			/* Color depth. (24-bit is the most common.) */
	uint32_t compression;		/* Compression. (0 == no compression) */
	uint32_t bmp_size;		/* Bitmap data size, in bytes. */
	uint32_t ppm_x;			/* Pixels per meter, X. */
	uint32_t ppm_y;			/* Pixels per meter, Y. */
	uint32_t colors_used;		/* Colors used. (0 on non-paletted bitmaps.) */
	uint32_t important_colors;	/* "Important" colors. (0 on non-paletted bitmaps.) */
} bmp_header_t;

#ifdef __cplusplus
}
#endif

#endif /* GENS_BMP_H */
