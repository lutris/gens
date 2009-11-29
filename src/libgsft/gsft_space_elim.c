/***************************************************************************
 * libgsft: Common Functions.                                              *
 * gsft_space_elim.c: Space elimination algorithm for ROM header names.    *
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

#include "gsft_space_elim.h"

// C includes.
#include <ctype.h>
#include <string.h>


/**
 * isGraphChar(): Determine if a character is a graphical character.
 * @param chr Character.
 * @return 0 if the character is a space; non-zero if the character is graphical.
 */
static inline int isGraphChar(char chr)
{
	return (isgraph(chr) || (chr & 0x80));
}


/**
 * gsft_space_elim(): Space elimination algorithm for ROM header names.
 * @param src Source name.
 * @param src_len Size of the source name.
 * @param dest Destination buffer. (Must be src_len+1 or greater!)
 * @return 0 on success; non-zero on error.
 */
int gsft_space_elim(const char *src, size_t src_len, char *dest)
{
	char *dest_ptr = dest;
	const char *src_ptr = src;
	int lastCharIsGraph = 0;
	
	for (size_t n = src_len; n != 0; n--)
	{
		char chr = *src_ptr++;
		if (chr == 0x00)
		{
			// NULL terminator encountered.
			// TODO: Should we translate it to a space?
			// The ROM header isn't supposed to contain any NULLs...
			break;
		}
		
		if (!lastCharIsGraph && !isGraphChar(chr))
		{
			// This is a space character, and the previous
			// character was not a space character.
			continue;
		}
		
		// This is not a space character.
		*dest_ptr++ = chr;
		lastCharIsGraph = isGraphChar(chr);
	}
	
	// Terminate the string.
	*dest_ptr = 0x00;
	
	// Check for a trailing space.
	if (dest_ptr > dest)
	{
		char last_chr = *(dest_ptr - 1);
		if (!isGraphChar(last_chr))
		{
			// Trailing space. Remove it.
			*(dest_ptr - 1) = 0x00;
		}
	}
	
	// Done.
	return 0;
}
