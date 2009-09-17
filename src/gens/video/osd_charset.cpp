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
#include "C64_charset.h"

// C includes.
#include <string.h>

// Character used if a character cannot be found.
static const uint8_t chr_err[8] = {0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA};

// Hashtable.
#include "libgsft/gsft_hashtable.hpp"
typedef GSFT_HASHTABLE<wchar_t, const uint8_t*> mapOsdCharSet_t;
typedef std::pair<wchar_t, const uint8_t*> pairOsdCharSet_t;
static mapOsdCharSet_t mapOsdCharSet;


/**
 * osd_charset_init(): Initialize the character set hashtable.
 * @param charset Character set to use.
 */
void osd_charset_init(const osd_char_t *charset)
{
	if (!charset)
		return;
	
	mapOsdCharSet.clear();
	
	// Add all characters to the map.
	for (; charset->chr != -1; charset++)
	{
		if (charset->data == NULL)
			continue;
		
		mapOsdCharSet.insert(pairOsdCharSet_t(charset->chr, charset->data));
	}
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
	
	if (mapOsdCharSet.empty())
		osd_charset_init(&C64_charset[0]);
	
	// TODO: UTF-8 translation.
	unsigned int len = strlen(str);
	const uint8_t *chr_data;
	
	for (unsigned int chr = 0; chr < len; chr++)
	{
		mapOsdCharSet_t::iterator chrIter = mapOsdCharSet.find(*str++);
		if (chrIter == mapOsdCharSet.end())
		{
			// Character not found.
			chr_data = &chr_err[0];
		}
		else
		{
			// Character found.
			chr_data = &(*chrIter).second[0];
		}
		
		for (unsigned int row = 0; row < 8; row++)
		{
			prerender_buf[row][chr] = chr_data[row];
		}
	}
	
	return len;
}
