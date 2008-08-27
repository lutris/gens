/***************************************************************************
 * Gens: Dummy File Compression Handler.                                    *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
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

// This compression handler doesn't actually do any decompression.
// Its purpose is to simplify the code in other parts of the program.

#include <zlib.h>
#include "compress.h"

/**
 * Dummy_Detect_Format(): Returns 1. All files can be handled by this module.
 * @param f File pointer of the file to check.
 * @return 1
 */
int Dummy_Detect_Format(FILE *f)
{
	return 1;
}

/**
 * Dummy_Get_First_File_Size(): Gets the filesize of the specified file.
 * @param filename Filename of the file to read.
 * @return Filesize, or 0 on error.
 */
int Dummy_Get_First_File_Size(const char *filename)
{
	int filesize;
	FILE *f;
	
	// Open the file.
	f = fopen(filename, "rb");
	// Return 0 if the file can't be read.
	if (!f)
		return 0;
	
	// Get the filesize.
	fseek(f, 0, SEEK_END);
	filesize = ftell(ROM_File);
	fclose(f);
	return filesize;
}

/**
 * Dummy_Get_First_File(): Gets the file contents.
 * @param filename Filename of the file to read.
 * @param buf Buffer to write the file to.
 * @param size Size of the buffer, in bytes.
 * @return Number of bytes read, or -1 on error.
 */
int Dummy_Get_First_File(const char *filename, void *buf, int size)
{
	int filesize;
	FILE *f;
	
	// Open the file.
	f = fopen(filename, "rb");
	// Return -1 if the file can't be read.
	if (!f)
		return 0;
	
	// Read the file.
	filesize = fread(buf, size, 1, f);
	
	// Close the file and return the number of bytes read.
	fclose(f);
	return filesize;
}
