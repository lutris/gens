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
#include <stdlib.h>
#include <stdio.h>

// Character used if a character cannot be found.
static const uint8_t chr_err[16] =
	{0x00, 0x38, 0x7C, 0x7C, 0xC6, 0x92, 0xF2, 0xE6,
	 0xFE, 0xE6, 0x7C, 0x7C, 0x38, 0x00, 0x00, 0x00};


// Character font data.
static void *chr_font_data[65536];	// Pointers to character data.
static uint8_t chr_font_flags[65536];	// Character flags.

// Character flags.
#define CHR_FLAG_HALFWIDTH	0
#define CHR_FLAG_FULLWIDTH	(1 << 0)


/**
 * osd_init(): Initialize the OSD font.
 */
void osd_init(void)
{
	memset(chr_font_data, 0x00, sizeof(chr_font_data));
	memset(chr_font_flags, 0x00, sizeof(chr_font_flags));
	
	// Initialize character data with the internal VGA character set.
	// TODO: Copy the data into memory so it can be replaced by external fonts.
	for (unsigned int chr = 0x20; chr < 0x80; chr++)
	{
		chr_font_data[chr] = (void*)&VGA_charset_ASCII[chr-0x20][0];
		chr_font_flags[chr] = CHR_FLAG_HALFWIDTH;
	}
	
	// TODO: Load font data.
}


/**
 * osd_end(): Shut down the OSD font.
 */
void osd_end(void)
{
	// Free the OSD font data.
	for (unsigned int chr = 0x80; chr < 65536; chr++)
		free(chr_font_data[chr]);
	
	memset(chr_font_data, 0x00, sizeof(chr_font_data));
	memset(chr_font_flags, 0x00, sizeof(chr_font_flags));
}


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
			// Not the start of a UTF-8 sequence. Assume it's ASCII.
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
		
		// Check if the character exists.
		if (wchr > 0xFFFF)
		{
			// Outside of BMP. Not found.
			chr_data = &chr_err[0];
		}
		else if (chr_font_data[wchr] == NULL)
		{
			// Character not found.
			chr_data = &chr_err[0];
		}
		else
		{
			// Character found.
			// TODO: Proper fullwidth handling.
			chr_data = (const uint8_t*)chr_font_data[wchr];
		}
		
		// Check for combining characters.
		// Unicode has the following combining characters:
		// * Combining Diacritical Marks (0300–036F)
		// * Combining Diacritical Marks Supplement (1DC0–1DFF)
		// * Combining Diacritical Marks for Symbols (20D0–20FF)
		// * Combining Half Marks (FE20–FE2F)
		// Reference: http://en.wikipedia.org/wiki/Combining_character
		if (chr_num > 0 &&
		    ((wchr >= 0x0300 && wchr <= 0x036F) ||
		     (wchr >= 0x1DC0 && wchr <= 0x1DFF) ||
		     (wchr >= 0x20D0 && wchr <= 0x20FF) ||
		     (wchr >= 0xFE20 && wchr <= 0xFE2F)))
		{
			// Unicode combining character.
			// OR the glyph with the previous character.
			// TODO: This isn't the perfect method, but it's good enough for now.
			chr_num--;
			for (unsigned int row = 0; row < 16; row++)
			{
				prerender_buf[row][chr_num] |= chr_data[row];
			}
		}
		else
		{
			// Regular character.
			for (unsigned int row = 0; row < 16; row++)
			{
				prerender_buf[row][chr_num] = chr_data[row];
			}
		}
		
		// Next character.
		chr_num++;
	}
	
	return chr_num;
}
