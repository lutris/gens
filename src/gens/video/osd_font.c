/***************************************************************************
 * Gens: On-Screen Display Font Handler.                                   *
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "osd_font.h"

// C includes.
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// VGA character set.
#include "VGA_charset.h"

// LOG_MSG() support.
#include "macros/log_msg.h"

// libgsft includes.
#include "libgsft/gsft_byteswap.h"

#ifdef GENS_ZLIB
#include <zlib.h>
#else
#define zf_osd f_osd
#define gzread(file, buf, len)	fread((buf), 1, (len), (file))
#define gzclose(file)		fclose(file)
#define gzeof(file)		feof(file)
#endif

// Needed for SetCurrentDirectory.
// TODO: Use libgsft/w32u/ once win32-unicode is merged to master.
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif /* _WIN32 */


// Character font data.
osd_ptr_t osd_font_data[65536];	// Pointers to character data.
uint8_t osd_font_flags[65536];	// Character flags.
static int osd_is_alloc = 0;	// Set to 1 if these have been allocated.


void osd_font_clear(void)
{
	if (osd_is_alloc)
	{
		// OSD has been allocated. Free everything.
		for (int chr = sizeof(osd_font_data)-1; chr >= 0; chr--)
			free(osd_font_data[chr].p_v);
	}
	
	memset(osd_font_data, 0x00, sizeof(osd_font_data));
	memset(osd_font_flags, 0x00, sizeof(osd_font_flags));
	osd_is_alloc = 1;
}


void osd_font_init_ASCII(void)
{
	if (!osd_is_alloc)
		osd_font_clear();
	
	for (unsigned int chr = 0x20; chr < 0x80; chr++)
	{
		if (osd_font_data[chr].p_v)
			free(osd_font_data[chr].p_v);
		
		osd_font_data[chr].p_v = malloc(16);
		memcpy(osd_font_data[chr].p_v, VGA_charset_ASCII[chr-0x20], 16);
		osd_font_flags[chr] = OSD_FLAG_HALFWIDTH;
	}
}


/**
 * osd_font_load(): Load an OSD font.
 * @param filename Filename of the OSD font.
 * @return 0 on success; non-zero on error.
 */
int osd_font_load(const char *filename)
{
#ifdef _WIN32
	// TODO: Use pSetCurrentDirectoryU once win32-unicode is merged to master.
	SetCurrentDirectory(PathNames.Gens_EXE_Path);
#endif
	
	FILE *f_osd = fopen(filename, "rb");
	if (!f_osd)
	{
		// Couldn't open the font file.
		LOG_MSG(gens, LOG_MSG_LEVEL_WARNING,
			"Couldn't open '%s': %s", filename, strerror(errno));
		return -1;
	}
	
#ifdef GENS_ZLIB
	gzFile zf_osd = gzdopen(fileno(f_osd), "rb");
	if (!zf_osd)
	{
		// ZLib error.
		LOG_MSG(gens, LOG_MSG_LEVEL_WARNING,
			"Couldn't gzdopen() '%s': %s", filename, strerror(errno));
	}
#endif
	
	// OSD file format stuff.
	static const uint8_t osd_header[] = {'M', 'D', 'F', 'o', 'n', 't', '1', 0x0A};
	static const uint8_t osd_eof[] = {'_', 'E', 'O', 'F'};
	uint8_t buf[32];
	
	// Get the file header.
	gzread(zf_osd, buf, 8);
	if (memcmp(osd_header, buf, sizeof(osd_header)) != 0)
	{
		// Invalid file header.
		gzclose(zf_osd);
		LOG_MSG(gens, LOG_MSG_LEVEL_WARNING,
			"'osd_font.bin' is not a valid Gens/GS OSD font.");
		return -2;
	}
	
	if (!osd_is_alloc)
		osd_font_clear();
	
	unsigned int num_chrs = 0;
	unsigned int num_chrs_nonbmp = 0;
	
	// Read the file.
	uint8_t chr_flag_buf[4];
	uint32_t chr;
	uint8_t flags;
	
	while (!gzeof(zf_osd))
	{
		gzread(zf_osd, chr_flag_buf, sizeof(chr_flag_buf));
		if (memcmp(osd_eof, chr_flag_buf, sizeof(osd_eof)) == 0)
		{
			// End of file.
			break;
		}
		
		// Convert the values from little-endian to CPU format.
		chr = (chr_flag_buf[0] | (chr_flag_buf[1] << 8) | (chr_flag_buf[2] << 16));
		flags = chr_flag_buf[3];
		
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
				gzread(zf_osd, buf, 16);
				if (flags & OSD_FLAG_FULLWIDTH)
					gzread(zf_osd, buf, 16);
				
				num_chrs_nonbmp++;
			}
		}
		
		// Check if this is a halfwidth or fullwidth character.
		if (!(flags & OSD_FLAG_FULLWIDTH))
		{
			// Halfwidth. Read 16 bytes.
			gzread(zf_osd, buf, 16);
			if (osd_font_data[chr].p_v != NULL)
			{
				// Character already exists. Skip it.
				continue;
			}
			
			// Copy this character to osd_font_data.
			osd_font_data[chr].p_u8 = (uint8_t*)malloc(16);
			memcpy(osd_font_data[chr].p_u8, buf, 16);
			osd_font_flags[chr] = flags;
		}
		else
		{
			// Fullwidth. Read 32 bytes.
			gzread(zf_osd, buf, 32);
			if (osd_font_data[chr].p_v != NULL)
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
			
			osd_font_data[chr].p_u16 = tmp;
			osd_font_flags[chr] = flags;
		}
		
		num_chrs++;
	}
	
	// Finished reading the file.
	gzclose(zf_osd);
	
	LOG_MSG(gens, LOG_MSG_LEVEL_INFO,
		"%d characters loaded from 'osd_font.bin'. (%d non-BMP characters skipped)",
		num_chrs, num_chrs_nonbmp);
	
	return 0;
}
