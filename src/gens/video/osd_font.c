/***************************************************************************
 * Gens: On-Screen Display Font Handler.                                   *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2010 by David Korth                                  *
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
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

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

// Win32 includes.
#ifdef _WIN32
#include "libgsft/w32u/w32u_windows.h"
#include "libgsft/w32u/w32u_libc.h"
#include "emulator/g_main.hpp"
#endif


// Character font data.
osd_ptr_t osd_font_data[65536];	// Pointers to character data.
uint8_t osd_font_flags[65536];	// Character flags.

// Stack of font memory blocks.
typedef struct _osd_font_mem_t
{
	void *ptr;
	struct _osd_font_mem_t *next;
} osd_font_mem_t;
osd_font_mem_t *osd_font_blocks_head = NULL;


static osd_font_mem_t* osd_font_alloc(size_t sz)
{
	osd_font_mem_t *cur = (osd_font_mem_t*)malloc(sizeof(osd_font_mem_t));
	cur->ptr = malloc(sz);
	cur->next = osd_font_blocks_head;
	osd_font_blocks_head = cur;
	return cur;
}


void osd_font_clear(void)
{
	memset(osd_font_data, 0x00, sizeof(osd_font_data));
	memset(osd_font_flags, 0x00, sizeof(osd_font_flags));
	
	// Free all allocated font memory blocks.
	osd_font_mem_t *cur = osd_font_blocks_head;
	while (cur != NULL)
	{
		osd_font_mem_t *next = cur->next;
		free(cur->ptr);
		free(cur);
		cur = next;
	}
}


void osd_font_init_ASCII(void)
{
	// Allocate a font memory block for ASCII character data.
	osd_font_mem_t *cur = osd_font_alloc(sizeof(VGA_charset_ASCII));
	
	// Copy the ASCII character data to the memory block.
	memcpy(cur->ptr, VGA_charset_ASCII, sizeof(VGA_charset_ASCII));
	
	// Set up osd_font_data.
	uint8_t *chr_ptr = (uint8_t*)cur->ptr;
	for (unsigned int chr = 0x20; chr < 0x80; chr++)
	{
		osd_font_data[chr].p_u8 = chr_ptr;
		osd_font_flags[chr] = OSD_FLAG_HALFWIDTH;
		chr_ptr += 16;
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
	// Make sure relative pathnames are handled correctly on Win32.
	pSetCurrentDirectoryU(PathNames.Gens_Save_Path);
#endif
	
	// Get the filesize.
	struct stat st_file;
	if (stat(filename, &st_file) != 0)
	{
		// Couldn't stat the file.
		LOG_MSG(gens, LOG_MSG_LEVEL_WARNING,
			"Couldn't stat() '%s': %s", filename, strerror(errno));
		return -1;
	}
	
	// TODO: Check for sysconf().
	int pg_size;
#if defined(_WIN32)
	// Win32 version.
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	pg_size = si.dwAllocationGranularity;
#elif defined(_SC_PAGESIZE)
	// sysconf() version.
	pg_size = sysconf(_SC_PAGESIZE);
#endif
	
	// Make sure the page size is at least 4096 bytes.
	if (pg_size < 4096)
		pg_size = 4096;
	
	// Calculate the block size. (Filesize rounded to the highest pagesize.)
	const size_t blk_size = (st_file.st_size & (~(pg_size - 1))) + pg_size;
	
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
	uint8_t buf[256];
	
	// Get the file header.
	gzread(zf_osd, buf, 8);
	if (memcmp(osd_header, buf, sizeof(osd_header)) != 0)
	{
		// Invalid file header.
		gzclose(zf_osd);
		LOG_MSG(gens, LOG_MSG_LEVEL_WARNING,
			"'%s' is not a valid Gens/GS OSD font.", filename);
		return -2;
	}
	
	// Get the font title.
	uint8_t font_title_len;
	gzread(zf_osd, &font_title_len, 1);
	if (font_title_len > 0)
	{
		gzread(zf_osd, buf, font_title_len);
		buf[font_title_len+1] = 0x00;
		LOG_MSG(gens, LOG_MSG_LEVEL_INFO,
			"Loading OSD font '%s': %s", filename, buf);
	}
	
	unsigned int num_chrs = 0;
	unsigned int num_chrs_nonbmp = 0;
	
	// Allocate a block.
	osd_font_mem_t *blk = osd_font_alloc(blk_size);
	uint8_t *blk_ptr = (uint8_t*)(blk->ptr);
	size_t blk_avail = blk_size;
	
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
			
			// Make sure space is available in the current block.
			if (blk_avail < 16)
			{
				// Current block is filled. Allocate a new block.
				blk = osd_font_alloc(blk_size);
				blk_ptr = (uint8_t*)(blk->ptr);
				blk_avail = blk_size;
			}
			
			// Copy the character data to the block.
			memcpy(blk_ptr, buf, 16);
			osd_font_data[chr].p_u8 = blk_ptr;
			osd_font_flags[chr] = flags;
			blk_ptr += 16;
			blk_avail -= 16;
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
			
			// Make sure space is available in the current block.
			if (blk_avail < 32)
			{
				// Current block is filled. Allocate a new block.
				blk = osd_font_alloc(blk_size);
				blk_ptr = (uint8_t*)(blk->ptr);
				blk_avail = blk_size;
			}
			
			// Copy the character data to the block.
#if GSFT_BYTEORDER == GSFT_LIL_ENDIAN
			// Little-endian. Copy the data as-is.
			memcpy(blk_ptr, buf, 32);
#else /* GSFT_BYTE_ORDER == GSFT_BIG_ENDIAN */
			// Big-endian. Byteswap the data.
			for (int i = 0; i < 32; i += 4)
			{
				blk_ptr[i + 0] = buf[i + 1];
				blk_ptr[i + 1] = buf[i + 0];
				blk_ptr[i + 2] = buf[i + 3];
				blk_ptr[i + 3] = buf[i + 2];
			}
#endif			
			osd_font_data[chr].p_u8 = blk_ptr;
			osd_font_flags[chr] = flags;
			blk_ptr += 32;
			blk_avail -= 32;
		}
		
		num_chrs++;
	}
	
	// Finished reading the file.
	gzclose(zf_osd);
	
	LOG_MSG(gens, LOG_MSG_LEVEL_INFO,
		"%d characters loaded from '%s'. (%d non-BMP characters skipped)",
		num_chrs, filename, num_chrs_nonbmp);
	
	return 0;
}
