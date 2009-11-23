/***************************************************************************
 * Gens: On-Screen Display Character Set.                                  *
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

#include "osd_charset.hpp"
#include "VGA_charset.h"

// C includes.
#include <string.h>

// Character used if a character cannot be found.
static const uint8_t chr_err[16] =
	{0x00, 0x38, 0x7C, 0x7C, 0xC6, 0x92, 0xF2, 0xE6,
	 0xFE, 0xE6, 0x7C, 0x7C, 0x38, 0x00, 0x00, 0x00};


/**
 * osd_charset_prerender(): Prerender a string.
 * @param str String to prerender.
 * @param prerender_buf Prerender buffer.
 * @return Number of characters rendered.
 */
int osd_charset_prerender(const char *str, uint8_t prerender_buf[8][1024])
{
	if (!str || !prerender_buf)
		return 0;
	
	const uint8_t *chr_data;
	const unsigned char *utf8str = reinterpret_cast<const unsigned char*>(str);
	unsigned int chr_num = 0;
	
	while (*utf8str)
	{
		wchar_t wchr;
		
		// Check if this is the start of a UTF-8 sequence.
		if (!(*utf8str & 0x80))
		{
			// Not the start of UTF-8. Assume it's ASCII.
			wchr = *utf8str++;
		}
		else
		{
			// Possibly the start of a UTF-8 sequence.
			
			// Check for a 2-byte character.
			if (utf8str[1] != 0x00 &&
			    ((utf8str[0] & 0xE0) == 0xC0) &&
			    ((utf8str[1] & 0xC0) == 0x80))
			{
				// 2-byte character.
				wchr = ((utf8str[0] & 0x1F) << 6) |
				       ((utf8str[1] & 0x3F));
				utf8str += 2;
			}
			else if (utf8str[1] != 0x00 && utf8str[2] != 0x00 &&
				 ((utf8str[0] & 0xF0) == 0xE0) &&
				 ((utf8str[1] & 0xC0) == 0x80) &&
				 ((utf8str[2] & 0xC0) == 0x80))
			{
				// 3-byte character.
				wchr = ((utf8str[0] & 0x1F) << 12) |
				       ((utf8str[1] & 0x3F) << 6) |
				       ((utf8str[2] & 0x3F));
				utf8str += 3;
			}
			else if (utf8str[1] != 0x00 && utf8str[2] != 0x00 && utf8str[3] != 0 &&
				 ((utf8str[0] & 0xF0) == 0xE0) &&
				 ((utf8str[1] & 0xC0) == 0x80) &&
				 ((utf8str[2] & 0xC0) == 0x80) &&
				 ((utf8str[3] & 0xC0) == 0x80))
			{
				// 4-byte character.
				wchr = ((utf8str[0] & 0x1F) << 18) |
				       ((utf8str[1] & 0x3F) << 12) |
				       ((utf8str[2] & 0x3F) << 6) |
				       ((utf8str[3] & 0x3F));
				utf8str += 4;
			}
			else
			{
				// Invalid UTF-8 sequence. Assume cp1252.
				wchr = *utf8str++;
			}
		}
		
		// Check if the page exists.
		if (wchr > 0xFFFF)
		{
			// Outside of BMP. Not found.
			chr_data = &chr_err[0];
		}
		else if (VGA_charset[wchr >> 8] == NULL)
		{
			// Page not found.
			chr_data = &chr_err[0];
		}
		else
		{
			// Page found.
			int page = (wchr >> 8) & 0xFF;
			int chr = wchr & 0xFF;
			
			chr_data = VGA_charset[page] + (16*chr);
		}
		
		for (unsigned int row = 0; row < 16; row++)
		{
			prerender_buf[row][chr_num] = chr_data[row];
		}
		
		// Next character.
		chr_num++;
	}
	
	return chr_num;
}
