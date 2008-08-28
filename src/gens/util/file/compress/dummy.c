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

#include "compress.h"
#include <string.h>


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
 * Dummy_Get_Num_Files(): Gets the number of files in the specified archive.
 * @param filename Filename of the archive.
 * @return Number of files, or 0 on error.
 */
int Dummy_Get_Num_Files(const char *filename)
{
	// Uncompressed or unsupported compression, so return 1.
	return 1;
}


/**
 * Dummy_Get_File_Info(): Gets information about all files in the specified archive.
 * @param filename Filename of the file to read.
 * @return Pointer to the first COMPRESS_FileInfo_t, or NULL on error.
 */
struct COMPRESS_FileInfo_t* Dummy_Get_File_Info(const char *filename)
{
	FILE *f;
	int filesize;
	struct COMPRESS_FileInfo_t *fileInfo;
	
	// The filename must be specified.
	if (!filename)
		return NULL;
	
	// Open the file.
	f = fopen(filename, "rb");
	// Return NULL if the file can't be read.
	if (!f)
		return NULL;
	
	// Get the filesize.
	fseek(f, 0, SEEK_END);
	filesize = ftell(f);
	fclose(f);
	
	// Create a FileInfo_t struct.
	fileInfo = (struct COMPRESS_FileInfo_t*)malloc(sizeof(struct COMPRESS_FileInfo_t));
	if (!fileInfo)
		return NULL;
	
	// Copy the filesize and filename to fileInfo.
	fileInfo->filesize = filesize;
	strncpy(fileInfo->filename, filename, 256);
	fileInfo->filename[255] = 0x00;
	
	// Only one file, so set the *next variable to NULL.
	fileInfo->next = NULL;
	
	// Done.
	return fileInfo;
}


/**
 * Dummy_Get_File(): Gets the file contents.
 * @param filename Filename of the file to read.
 * @param fileInfo Information about the file to extract. (Unused in the Dummy handler.)
 * @param buf Buffer to write the file to.
 * @param size Size of the buffer, in bytes.
 * @return Number of bytes read, or -1 on error.
 */
int Dummy_Get_File(const char *filename, const struct COMPRESS_FileInfo_t *fileInfo, void *buf, int size)
{
	int filesize;
	FILE *f;
	
	// All parameters (except fileInfo) must be specified.
	if (!filename || !buf || !size)
		return 0;
	
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
