/***************************************************************************
 * Gens: Genesis Save RAM handler.                                         *
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

#include "sram.h"
#include "rom.hpp"	/* ROM_Filename[] */

#include "emulator/g_main.hpp"

// SRAM variables.
#include "gens_core/mem/mem_m68k.h"

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

// SRAM directory.
char SRAM_Dir[GENS_PATH_MAX] = "";


/**
 * SRAM_GetFilename(): Get the filename of the SRAM file.
 * @param buf [out] Buffer for the filename.
 * @param len [out] Length of the buffer.
 */
static inline void SRAM_GetFilename(char *buf, size_t len)
{
	if (ROM_Filename[0] == 0x00)	// (strlen(ROM_Filename) == 0)
	{
		// ROM filename is empty.
		buf[0] = 0x00;
		return;
	}
	
	szprintf(buf, len, "%s%s.srm", SRAM_Dir, ROM_Filename);
}


/**
 * SRAM_Load(): Load the SRAM file.
 * @return 0 on success; non-zero on error.
 */
int SRAM_Load(void)
{
	// Reset SRAM before loading.
	// SRAM is reset to 0xFF, since that's how SRAM is initialized from the factory.
	memset(SRAM, 0xFF, sizeof(SRAM));
	
#ifdef GENS_OS_WIN32
	// Make sure relative pathnames are handled correctly on Win32.
	pSetCurrentDirectoryU(PathNames.Gens_Save_Path);
#endif
	
	// Create the SRAM filename.
	char filename[1024];
	SRAM_GetFilename(filename, sizeof(filename));
	if (filename[0] == 0x00)
		return -1;
	
	FILE *SRAM_File = fopen(filename, "rb");
	if (!SRAM_File)
		return -2;
	
	fread(SRAM, sizeof(SRAM), 1, SRAM_File);
	fclose(SRAM_File);
	
	vdraw_text_printf(2000, "SRAM loaded from %s", filename);
	return 0;
}


/**
 * SRAM_Save(): Save the SRAM file.
 * @return 0 on success; non-zero on error.
 */
int SRAM_Save(void)
{
	int size_to_save, i;
	
	i = (64 * 1024) - 1;
	while ((i >= 0) && (SRAM[i] == 0xFF))
		i--;
	
	if (i < 0)
		return -1;
	
	i++;
	
	size_to_save = 1;
	while (i > size_to_save)
		size_to_save <<= 1;
	
#ifdef GENS_OS_WIN32
	// Make sure relative pathnames are handled correctly on Win32.
	pSetCurrentDirectoryU(PathNames.Gens_Save_Path);
#endif
	
	// Create the SRAM filename.
	char filename[1024];
	SRAM_GetFilename(filename, sizeof(filename));
	if (filename[0] == 0x00)
		return -1;
	
	FILE *SRAM_File = fopen(filename, "wb");
	if (!SRAM_File)
		return -3;
	
	fwrite(SRAM, size_to_save, 1, SRAM_File);
	fclose(SRAM_File);
	
	vdraw_text_printf(2000, "SRAM saved to %s", filename);
	return 0;
}
