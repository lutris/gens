/***************************************************************************
 * Gens: Zip File Compression Handler.                                     *
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

#include <stdlib.h>
#include <string.h>
#include "compress.h"
#include "unzip.h"
#include "ui-common.h"


/**
 * Zip_Detect_Format(): Detects if a file is in Zip format.
 * @param f File pointer of the file to check.
 * @return 0 if this is not in Zip format; non-zero if it is.
 */
int Zip_Detect_Format(FILE *f)
{
	// Magic Number for Zip:
	// First two bytes: "PK\x03\x04"
	unsigned char buf[4];
	fseek(f, 0, SEEK_SET);
	fread(buf, 4, sizeof(unsigned char), f);
	return (buf[0] == 'P' && buf[1] == 'K' &&
		buf[2] == 0x03 && buf[3] == 0x04);
}


/**
 * Zip_Get_Num_Files(): Gets the number of files in the specified archive.
 * @param filename Filename of the archive.
 * @return Number of files, or 0 on error.
 */
int Zip_Get_Num_Files(const char *filename)
{
	unzFile f;
	int i, numFiles;
	
	// Filename must be specified.
	if (!filename)
		return 0;
	
	// Open the Zip file.
	f = unzOpen(filename);
	if (!f)
		return 0;
	
	// Count the number of files in the archive.
	numFiles = 0;
	i = unzGoToFirstFile(f);
	while (i == UNZ_OK)
	{
		if (i == UNZ_OK)
			numFiles++;
		i = unzGoToNextFile(f);
	}
	unzClose(f);
	
	// Return the number of files found.
	return numFiles;
}


/**
 * Zip_Get_First_File_Info(): Gets information about the first file in the specified archive.
 * @param filename Filename of the archive.
 * @param retFileInfo Struct to store information about the file.
 * @return 1 on success; 0 on error.
 */
int Zip_Get_First_File_Info(const char *filename, struct COMPRESS_FileInfo_t *retFileInfo)
{
	unzFile f;
	unz_file_info zinfo;
	int i, extpos;
	char ROMFileName[132];
	
	// Both parameters must be specified.
	if (!filename || !retFileInfo)
		return 0;
	
	// Open the Zip file.
	f = unzOpen(filename);
	if (!f)
		return 0;
	
	// Find the first ROM file in the Zip archive.
	i = unzGoToFirstFile(f);
	while (i == UNZ_OK)
	{
		unzGetCurrentFileInfo(f, &zinfo, ROMFileName, 128, NULL, 0, NULL, 0);
		// The file extension of the file in the Zip file must match one of these
		// in order to be considered a ROM, since extracting each file to check
		// based on contents is too resource-intensive.
		extpos = strlen(ROMFileName) - 4;
		if ((!strncasecmp(".smd", &ROMFileName[extpos], 4)) ||
		    (!strncasecmp(".bin", &ROMFileName[extpos], 4)) ||
		    (!strncasecmp(".gen", &ROMFileName[extpos], 4)) ||
		    (!strncasecmp(".32x", &ROMFileName[extpos], 4)))
		{
			// Store the ROM file informatio.
			retFileInfo->filesize = zinfo.uncompressed_size;
			strncpy(retFileInfo->filename, ROMFileName, 132);
			retFileInfo->filename[131] = 0x00;
			break;
		}
		
		i = unzGoToNextFile(f);
	}
	unzClose(f);
	
	if ((i != UNZ_END_OF_LIST_OF_FILE && i != UNZ_OK) || retFileInfo->filesize == 0)
	{
		UI_MsgBox("No Genesis or 32X ROM file found in ZIP archive.", "ZIP File Error");
		return 0;
	}
	
	// Return successfully.
	return 1;
}


/**
 * Zip_Get_File_Info(): Gets information about all files in the specified archive.
 * @param filename Filename of the archive.
 * @return Pointer to the first COMPRESS_FileInfo_t, or NULL on error.
 */
struct COMPRESS_FileInfo_t*  Zip_Get_File_Info(const char *filename)
{
	unzFile f;
	unz_file_info zinfo;
	int i;
	char ROMFileName[132];
	struct COMPRESS_FileInfo_t *fileInfo_1 = NULL;
	struct COMPRESS_FileInfo_t *fileInfo_cur = NULL;
	struct COMPRESS_FileInfo_t *fileInfo_tmp = NULL;
	
	// The filename must be specified.
	if (!filename)
		return NULL;
	
	// Open the Zip file.
	f = unzOpen(filename);
	if (!f)
		return 0;
	
	// Find the first ROM file in the Zip archive.
	i = unzGoToFirstFile(f);
	while (i == UNZ_OK)
	{
		unzGetCurrentFileInfo(f, &zinfo, ROMFileName, 128, NULL, 0, NULL, 0);
		
		// Create a new FileInfo_t struct for this file.
		if (fileInfo_cur == NULL)
		{
			// First file.
			fileInfo_cur = (struct COMPRESS_FileInfo_t*)malloc(sizeof(struct COMPRESS_FileInfo_t));
			if (!fileInfo_cur)
				return NULL;
			fileInfo_1 = fileInfo_cur;
		}
		else
		{
			// Not first file.
			fileInfo_cur->next = (struct COMPRESS_FileInfo_t*)malloc(sizeof(struct COMPRESS_FileInfo_t));
			if (!fileInfo_cur->next)
			{
				// Error allocating a fileInfo struct.
				// Free all already-allocated fileInfo structs.
				fileInfo_cur = fileInfo_1;
				while (fileInfo_cur)
				{
					fileInfo_tmp = fileInfo_cur->next;
					free(fileInfo_cur);
					fileInfo_cur = fileInfo_tmp;
				}
				return NULL;
			}
			fileInfo_cur = fileInfo_cur->next;
		}
		
		// Store the ROM file information.
		fileInfo_cur->filesize = zinfo.uncompressed_size;
		strncpy(fileInfo_cur->filename, ROMFileName, 132);
		fileInfo_cur->filename[132] = 0x00;
		
		// Go to the next file.
		i = unzGoToNextFile(f);
	}
	unzClose(f);
	
	// Set the next pointer in the current fileInfo to NULL.
	fileInfo_cur->next = NULL;
	
	// Return the pointer to the first COMPRESS_FileInfo_t.
	return fileInfo_1;
}


/**
 * Zip_Get_File(): Gets the specified file from the specified archive.
 * @param filename Filename of the archive.
 * @param fileInfo Information about the file to extract.
 * @param buf Buffer to write the file to.
 * @param size Size of the buffer, in bytes.
 * @return Number of bytes read, or -1 on error.
 */
int Zip_Get_File(const char *filename, const struct COMPRESS_FileInfo_t *fileInfo, void *buf, int size)
{
	unzFile f;
	int zResult;
	char tmp[64];
	
	// All parameters must be specified.
	if (!filename || !fileInfo || !buf || !size)
		return 0;
	
	f = unzOpen(filename);
	if (!f)
		return 0;
	
	// Locate the ROM in the Zip file.
	if (unzLocateFile(f, fileInfo->filename, 1) != UNZ_OK ||
	    unzOpenCurrentFile(f) != UNZ_OK)
	{
		UI_MsgBox("Error loading the ROM file from the ZIP archive.", "ZIP File Error");
		unzClose(f);
		return -1;
	}
	
	// Decompress the ROM.
	zResult = unzReadCurrentFile(f, buf, size);
	unzClose(f);
	if ((zResult <= 0) || (zResult != size))
	{
		strcpy(tmp, "Error in ZIP file: \n");
		
		switch (zResult)
		{
			case UNZ_ERRNO:
				strcat(tmp, "Unknown...");
				break;
			case UNZ_EOF:
				strcat(tmp, "Unexpected end of file.");
				break;
			case UNZ_PARAMERROR:
				strcat(tmp, "Parameter error.");
				break;
			case UNZ_BADZIPFILE:
				strcat(tmp, "Bad ZIP file.");
				break;
			case UNZ_INTERNALERROR:
				strcat(tmp, "Internal error.");
				break;
			case UNZ_CRCERROR:
				strcat(tmp, "CRC error.");
				break;
		}
		UI_MsgBox(tmp, "ZIP File Error");
		return -1;
	}
	
	// Return the filesize.
	return size;
}
