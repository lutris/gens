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

#include "compress.h"
#include <string.h>
#include <zlib.h>


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
 * GZip_Get_Num_Files(): Gets the number of files in the specified archive.
 * @param filename Filename of the archive.
 * @return Number of files, or 0 on error.
 */
int GZip_Get_Num_Files(const char *filename)
{
	// GZip files always contain a single file.
	return 1;
}


/**
 * GZip_Get_First_File_Info(): Gets information about the first file in the specified archive.
 * @param filename Filename of the archive.
 * @param retFileInfo Struct to store information about the file. (Unused in the GZip handler.)
 * @return Filesize, or 0 on error.
 */
int GZip_Get_First_File_Info(const char *filename, struct COMPRESS_FileInfo_t *retFileInfo)
{
	gzFile gzfd;
	char buf[1024];
	
	// Both parameters must be specified.
	if (!filename || !retFileInfo)
		return 0;
	
	gzfd = gzopen(filename, "rb");
	if (!gzfd)
	{
		// Error obtaining a GZip file descriptor.
		return 0;
	}
	
	// Read through the GZip file until we hit an EOF.
	retFileInfo->filesize = 0;
	while (!gzeof(gzfd))
	{
		retFileInfo->filesize += gzread(gzfd, buf, 1024);
	}
	
	// Copy the filename.
	strncpy(retFileInfo->filename, filename, 256);
	retFileInfo->filename[255] = 0x00;
	
	// Close the GZip fd and return successfully.
	gzclose(gzfd);
	return 1;
}


/**
 * GZip_Get_File(): Gets the specified file from the specified archive.
 * @param filename Filename of the archive.
 * @param fileInfo Information about the file to extract. (Unused in the GZip handler.)
 * @param buf Buffer to write the file to.
 * @param size Size of the buffer, in bytes.
 * @return Number of bytes read, or -1 on error.
 */
int GZip_Get_File(const char *filename, const struct COMPRESS_FileInfo_t *fileInfo, void *buf, int size)
{
	gzFile gzfd;
	int retval;
	
	// All parameters (except fileInfo) must be specified.
	if (!filename || !buf || !size)
		return 0;
	
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
