/***************************************************************************
 * libgsft: Common Functions.                                              *
 * gsft_byteswap.h: Byteswapping functions.                                *
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

#include "gsft_byteswap.h"

/**
 * __byte_swap_16_array(): Swaps an array of bytes in 16-bit chunks.
 * @param ptr Pointer to bytes.
 * @param n Number of bytes to swap.
 */
void __byte_swap_16_array(void *ptr, int n)
{
	int i;
	unsigned char x;
	unsigned char *cptr;
	cptr = (unsigned char*)ptr;
	
	for (i = 0; i < n; i += 2)
	{
		x = cptr[i];
		cptr[i] = cptr[i + 1];
		cptr[i + 1] = x;
	}
}
