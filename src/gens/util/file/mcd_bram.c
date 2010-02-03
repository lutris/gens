/***************************************************************************
 * Gens: SegaCD Backup RAM handler.                                        *
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

#include "mcd_bram.h"
#include "save.hpp"	/* State_Dir[] */
#include "rom.hpp"	/* ROM_Filename[] */

#include "emulator/g_main.hpp"

// BRAM variables.
#include "gens_core/mem/mem_m68k_cd.h"
#include "gens_core/mem/mem_s68k.h"

// Text drawing functions.
#include "video/vdraw_text.hpp"

// C includes.
#include <stdio.h>
#include <string.h>

// libgsft includes.
#include "libgsft/gsft_szprintf.h"

// Win32 Unicode Translation Layer.
#ifdef GENS_OS_WIN32
#include "libgsft/w32u/w32u_windows.h"
#include "libgsft/w32u/w32u_libc.h"
#endif


/**
 * BRAM_Format(): Format the SegaCD Backup RAM.
 */
static void BRAM_Format(void)
{
	// Format the internal backup RAM.
	static const uint8_t BRAM_Header[0x40] =
	{
		0x5F, 0x5F, 0x5F, 0x5F, 0x5F, 0x5F, 0x5F, 0x5F,
		0x5F, 0x5F, 0x5F, 0x00, 0x00, 0x00, 0x00, 0x40,
		0x00, 0x7D, 0x00, 0x7D, 0x00, 0x7D, 0x00, 0x7D,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		
		'S', 'E', 'G', 'A', 0x5F, 'C', 'D', 0x5F, 'R', 'O', 'M', 0x00, 0x01, 0x00, 0x00, 0x00,
		'R', 'A', 'M', 0x5F, 'C', 'A', 'R', 'T', 'R', 'I', 'D', 'G', 'E', 0x5F, 0x5F, 0x5F
	};
	memset(Ram_Backup, 0x00, 0x1FC0);
	memcpy(&Ram_Backup[0x1FC0], BRAM_Header, sizeof(BRAM_Header));
	
	// SegaCD cartridge memory.
	memset(Ram_Backup_Ex, 0x00, sizeof(Ram_Backup_Ex));
	const unsigned int bram_size = ((8 << BRAM_Ex_Size) * 1024);
	memcpy(&Ram_Backup_Ex[bram_size - 0x40], BRAM_Header, sizeof(BRAM_Header));
	
	// Calculate the number of free blocks.
	const unsigned int free_blocks = ((bram_size / 64) - 3);
	const uint8_t free_blocks_hi = ((free_blocks >> 8) & 0xFF);
	const uint8_t free_blocks_lo = (free_blocks & 0xFF);
	
	// Write the number of free blocks.
	const unsigned int free_block_pos = (bram_size - 0x30);
	Ram_Backup_Ex[free_block_pos] = free_blocks_hi;
	Ram_Backup_Ex[free_block_pos + 1] = free_blocks_lo;
	Ram_Backup_Ex[free_block_pos + 2] = free_blocks_hi;
	Ram_Backup_Ex[free_block_pos + 3] = free_blocks_lo;
	Ram_Backup_Ex[free_block_pos + 4] = free_blocks_hi;
	Ram_Backup_Ex[free_block_pos + 5] = free_blocks_lo;
	Ram_Backup_Ex[free_block_pos + 6] = free_blocks_hi;
	Ram_Backup_Ex[free_block_pos + 7] = free_blocks_lo;
}


/**
 * BRAM_GetFilename(): Get the filename of the BRAM file.
 * @param buf [out] Buffer for the filename.
 * @param len [out] Length of the buffer.
 */
static inline void BRAM_GetFilename(char *buf, size_t len)
{
	if (ROM_Filename[0] == 0x00)	// (strlen(ROM_Filename) == 0)
	{
		// ROM filename is empty.
		buf[0] = 0x00;
		return;
	}
	
	szprintf(buf, len, "%s%s.brm", State_Dir, ROM_Filename);
}


/**
 * BRAM_Load(): Load the BRAM file.
 * @return 0 on success; non-zero on error.
 */
int BRAM_Load(void)
{
	// Format the SegaCD BRAM first.
	BRAM_Format();
	
#ifdef GENS_OS_WIN32
	// Make sure relative pathnames are handled correctly on Win32.
	pSetCurrentDirectoryU(PathNames.Gens_Save_Path);
#endif
	
	// Create the BRAM filename.
	char filename[1024];
	BRAM_GetFilename(filename, sizeof(filename));
	if (filename[0] == 0x00)
		return -1;
	
	FILE *BRAM_File = fopen(filename, "rb");
	if (!BRAM_File)
		return -2;
	
	// Read BRAM.
	fread(Ram_Backup, 1, sizeof(Ram_Backup), BRAM_File);
	if (BRAM_Ex_State & 0x100)
		fread(Ram_Backup_Ex, 1, (8 << BRAM_Ex_Size) * 1024, BRAM_File);
	
	fclose(BRAM_File);
	
	vdraw_text_printf(2000, "BRAM loaded from %s", filename);
	return 0;
}


/**
 * BRAM_Save(): Save the BRAM file.
 * @return 0 on success; non-zero on error.
 */
int BRAM_Save(void)
{
#ifdef GENS_OS_WIN32
	// Make sure relative pathnames are handled correctly on Win32.
	pSetCurrentDirectoryU(PathNames.Gens_Save_Path);
#endif
	
	char filename[1024];
	BRAM_GetFilename(filename, sizeof(filename));
	if (filename[0] == 0x00)
		return -1;
	
	FILE *BRAM_File = fopen(filename, "wb");
	if (!BRAM_File)
		return -2;
	
	// Read BRAM.
	fwrite(Ram_Backup, 1, sizeof(Ram_Backup), BRAM_File);
	if (BRAM_Ex_State & 0x100)
		fwrite(Ram_Backup_Ex, 1, (8 << BRAM_Ex_Size) * 1024, BRAM_File);
	
	fclose(BRAM_File);
	
	vdraw_text_printf(2000, "BRAM saved to %s", filename);
	return 0;
}
