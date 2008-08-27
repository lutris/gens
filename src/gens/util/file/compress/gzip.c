/***************************************************************************
 * Gens: GZip File Compression Handler.                                    *
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

#include <zlib.h>
#include "compress.h"

/**
 * GZip_Detect_Format(): Detects if a file is in GZip format.
 * @param f File pointer of the file to check.
 * @return 0 if this is not in GZip format; non-zero if it is.
 */
int GZip_Detect_Format(FILE *f)
{
	// Magic Number for GZip:
	// First two bytes: "\x1F\x8B"
	unsigned char buf[2];
	fseek(f, 0, SEEK_SET);
	fread(buf, 2, sizeof(unsigned char), f);
	return (buf[0] == 0x1F && buf[1] == 0x8B);
}

/**
 * GZip_Get_First_File_Size(): Gets the filesize of the first file in the specified archive.
 * @param filename Filename of the archive.
 * @return Filesize, or 0 on error.
 */
int GZip_Get_First_File_Size(const char *filename)
{
	gzFile gzfd;
	char buf[1024];
	int filesize = 0;
	
	gzfd = gzopen(filename, "rb");
	if (!gzfd)
	{
		// Error obtaining a GZip file descriptor.
		return 0;
	}
	
	// Read through the GZip file until we hit an EOF.
	while (!gzeof(gzfd))
	{
		filesize += gzread(gzfd, buf, 1024);
	}
	
	// Close the GZip fd and return the filesize.
	gzclose(gzfd);
	return filesize;
}

/**
 * GZip_Get_First_File(): Gets the first file from the specified archive.
 * @param filename Filename of the archive.
 * @param buf Buffer to write the file to.
 * @param size Size of the buffer, in bytes.
 * @return Number of bytes read, or -1 on error.
 */
int GZip_Get_First_File(const char *filename, void *buf, int size)
{
	gzFile gzfd;
	int retval;
	
	gzfd = gzopen(filename, "rb");
	if (!gzfd)
	{
		// Error obtaining a GZip file descriptor.
		return -1;
	}
	
	// Decompress the GZip file into memory.
	retval = gzread(gzfd, buf, size);
	gzclose(gzfd);
	return retval;
}
