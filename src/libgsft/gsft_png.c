/***************************************************************************
 * libgsft: Common functions.                                              *
 * gsft_png.c: PNG handling functions.                                     *
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

#include "gsft_png.h"
#include "gsft_png_dll.h"


/**
 * gsft_png_user_read_data(): libpng user-specified read data function.
 * Used to read PNG data from memory instead of from a file.
 * @param png_ptr Pointer to the PNG information struct.
 * @param png_bytep Pointer to memory to write PNG data to.
 * @param length Length of data requested.
 */
void gsft_png_user_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
	// Get the pointer to the gsft_png_mem_t struct.
	gsft_png_mem_t *png_mem = (gsft_png_mem_t*)ppng_get_io_ptr(png_ptr);
	if (!png_mem)
		return;
	
	// Make sure there's enough data available.
	if (png_mem->pos + length > png_mem->length)
	{
		// Not enough data is available.
		// TODO: This may still result in a crash. Use longjmp()?
		
		// Zero the buffer.
		memset(data, 0x00, length);
		
		// Return the rest of the buffer.
		length = png_mem->length - png_mem->pos;
		if (length <= 0)
			return;
	}
	
	// Copy the data.
	memcpy(data, &(png_mem->data[png_mem->pos]), length);
	
	// Increment the data position.
	png_mem->pos += length;
}
