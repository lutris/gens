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
#include <errno.h>

// LOG_MSG() support.
#include "macros/log_msg.h"

// libgsft includes.
#include "libgsft/gsft_byteswap.h"

// Needed for SetCurrentDirectory.
// TODO: Use libgsft/w32u/ once win32-unicode is merged to master.
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif /* _WIN32 */

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

typedef union
{
	const uint8_t *p_u8;
	const uint16_t *p_u16;
	const void *p_v;
} chr_ptr_t;


/**
 * osd_init(): Initialize the OSD font.
 */
void osd_init(void)
{
	memset(chr_font_data, 0x00, sizeof(chr_font_data));
	memset(chr_font_flags, 0x00, sizeof(chr_font_flags));
	
	// Initialize character data with the internal VGA character set.
	for (unsigned int chr = 0x20; chr < 0x80; chr++)
	{
		chr_font_data[chr] = malloc(16);
		memcpy(chr_font_data[chr], VGA_charset_ASCII[chr-0x20], 16);
		chr_font_flags[chr] = CHR_FLAG_HALFWIDTH;
	}
	
#ifdef _WIN32
	// TODO: Use pSetCurrentDirectoryU once win32-unicode is merged to master.
	SetCurrentDirectory(PathNames.Gens_EXE_Path);
#endif
	
	// Load font data from "osd_font.bin"
	// TODO: Make it customizable?
	// TODO: GZipped file support?
	FILE *f_osd = fopen("osd_font.bin", "rb");
	if (!f_osd)
	{
		// Couldn't open the font file.
		LOG_MSG(gens, LOG_MSG_LEVEL_WARNING,
			"Couldn't open 'osd_font.bin': %s", strerror(errno));
		LOG_MSG(gens, LOG_MSG_LEVEL_WARNING,
			"OSD will only display ASCII characters.");
		return;
	}
	
	// OSD file format stuff.
	static const uint8_t osd_header[] = {'M', 'D', 'F', 'o', 'n', 't', '1', 0x0A};
	static const uint8_t osd_eof[] = {'_', 'E', 'O', 'F'};
	uint8_t buf[32];
	
	// Get the file header.
	fread(buf, 1, 8, f_osd);
	if (memcmp(osd_header, buf, sizeof(osd_header)) != 0)
	{
		// Invalid file header.
		fclose(f_osd);
		LOG_MSG(gens, LOG_MSG_LEVEL_WARNING,
			"'osd_font.bin' is not a valid Gens/GS OSD font.");
		LOG_MSG(gens, LOG_MSG_LEVEL_WARNING,
			"OSD will only display ASCII characters.");
		return;
	}
	
	unsigned int num_chrs = 0;
	unsigned int num_chrs_nonbmp = 0;
	
	// Read the file.
	while (!feof(f_osd))
	{
		uint32_t chr;
		uint16_t flags;
		
		fread(&chr, 1, sizeof(chr), f_osd);
		if (memcmp(osd_eof, &chr, sizeof(osd_eof)) == 0)
		{
			// End of file.
			break;
		}
		
		fread(&flags, 1, sizeof(flags), f_osd);
		
		// Convert the values from little-endian to CPU format.
		chr = le32_to_cpu(chr);
		flags = le16_to_cpu(flags);
		
		if (chr > 0xFFFF)
		{
			// Characters outside of the BMP aren't supported.
			if (chr > 0x10FFFF)
			{
				// Invalid Unicode character. Font may be corrupted.
				LOG_MSG(gens, LOG_MSG_LEVEL_WARNING,
					"'osd_font.bin' contains invalid character U+%04X. Font processing aborted.", chr);
				break;
			}
			else
			{
				// Simply skip the character.
				fseek(f_osd, 16, SEEK_CUR);
				if (flags & CHR_FLAG_FULLWIDTH)
					fseek(f_osd, 16, SEEK_CUR);
				
				num_chrs_nonbmp++;
			}
		}
		
		// Check if this is a halfwidth or fullwidth character.
		if (!(flags & CHR_FLAG_FULLWIDTH))
		{
			// Halfwidth. Read 16 bytes.
			fread(buf, 1, 16, f_osd);
			if (chr_font_data[chr] != NULL)
			{
				// Character already exists. Skip it.
				continue;
			}
			
			// Copy this character to chr_font_data.
			chr_font_data[chr] = malloc(16);
			memcpy(chr_font_data[chr], buf, 16);
			chr_font_flags[chr] = flags;
		}
		else
		{
			// Fullwidth. Read 32 bytes.
			fread(buf, 1, 32, f_osd);
			if (chr_font_data[chr] != NULL)
			{
				// Character already exists. Skip it.
				continue;
			}
			
			// Make sure the character is byteswapped correctly.
			uint16_t *tmp = (uint16_t*)malloc(16 * sizeof(tmp));
			memcpy(tmp, buf, 32);
			for (int i = 0; i < 16; i++)
			{
				tmp[i] = le16_to_cpu(tmp[i]);
			}
			
			chr_font_data[chr] = tmp;
			chr_font_flags[chr] = flags;
		}
		
		num_chrs++;
	}
	
	// Finished reading the file.
	fclose(f_osd);
	
	LOG_MSG(gens, LOG_MSG_LEVEL_INFO,
		"%d characters loaded from 'osd_font.bin'. (%d non-BMP characters skipped)",
		num_chrs, num_chrs_nonbmp);
}


/**
 * osd_end(): Shut down the OSD font.
 */
void osd_end(void)
{
	// Free the OSD font data.
	for (unsigned int chr = 0; chr < 65536; chr++)
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
	
	chr_ptr_t chr_data;
	const unsigned char *utf8str = reinterpret_cast<const unsigned char*>(str);
	unsigned int chr_num = 0;
	
	while (*utf8str && chr_num < 1023)
	{
		wchar_t wchr;
		bool is_fullwidth = false;
		
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
			chr_data.p_u8 = &chr_err[0];
		}
		else if (chr_font_data[wchr] == NULL)
		{
			// Character not found.
			chr_data.p_u8 = &chr_err[0];
		}
		else
		{
			// Character found.
			chr_data.p_v = chr_font_data[wchr];
			is_fullwidth = (chr_font_flags[wchr] & CHR_FLAG_FULLWIDTH);
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
			// TODO: Check for fullwidth combining characters.
			chr_num--;
			for (unsigned int row = 0; row < 16; row++)
			{
				prerender_buf[row][chr_num] |= chr_data.p_u8[row];
			}
		}
		else
		{
			// Regular character.
			if (!is_fullwidth)
			{
				// Halfwidth character.
				for (unsigned int row = 0; row < 16; row++)
				{
					prerender_buf[row][chr_num] = chr_data.p_u8[row];
				}
			}
			else
			{
				// Fullwidth character.
				for (unsigned int row = 0; row < 16; row++)
				{
					prerender_buf[row][chr_num] = chr_data.p_u16[row] >> 8;
					prerender_buf[row][chr_num + 1] = chr_data.p_u16[row] & 0xFF;
				}
				chr_num++;
			}
		}
		
		// Next character.
		chr_num++;
	}
	
	return chr_num;
}
