/***************************************************************************
 * Gens: 7z Decompressor.                                                  *
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

#include "md_7z.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ui/gens_ui.hpp"

// 7z includes.
#include "lzma/7zCrc.h"
#include "lzma/7zFile.h"
#include "lzma/7zVersion.h"

#include "lzma/Archive/7z/7zAlloc.h"
#include "lzma/Archive/7z/7zExtract.h"
#include "lzma/Archive/7z/7zIn.h"

// 7z decompressor functions.
static int decompressor_7z_detect_format(FILE *zF);
static file_list_t* decompressor_7z_get_file_info(FILE *zF, const char* filename);
static int decompressor_7z_get_file(FILE *zF, const char* filename,
				    file_list_t *file_list,
				    unsigned char *buf, const int size);

// 7z decompressor struct.
decompressor_t decompressor_7z =
{
	.detect_format	= decompressor_7z_detect_format,
	.get_file_info	= decompressor_7z_get_file_info,
	.get_file	= decompressor_7z_get_file
};


/**
 * decompressor_7z_detect_format(): Detect if this file can be handled by this decompressor.
 * @param zF Open file handle.
 * @return Non-zero if this file can be handled; 0 if it can't be.
 */
static int decompressor_7z_detect_format(FILE *zF)
{
	// Magic Number for 7z:
	// First six bytes: "7z\xBC\xAF\x27\x1C"
	static const unsigned char _7z_magic[6] = {'7', 'z', 0xBC, 0xAF, 0x27, 0x1C};
	
	unsigned char buf[sizeof(_7z_magic)];
	fseek(zF, 0, SEEK_SET);
	fread(buf, sizeof(unsigned char), sizeof(_7z_magic), zF);
	return (memcmp(buf, _7z_magic, sizeof(_7z_magic)) == 0);
}


/**
 * decompressor_7z_get_file_info(): Get information from all files in the archive.
 * @param zF Open file handle.
 * @param filename Filename of the archive.
 * @return Pointer to the first file in the list, or NULL on error.
 */
static file_list_t* decompressor_7z_get_file_info(FILE *zF, const char* filename)
{
	// Unused parameters.
	((void)zF);
	
	CFileInStream archiveStream;
	CLookToRead lookStream;
	CSzArEx db;
	SRes res;
	ISzAlloc allocImp;
	ISzAlloc allocTempImp;
	
	// Open the 7z file.
	if (InFile_Open(&archiveStream.file, filename))
		return NULL;
	
	FileInStream_CreateVTable(&archiveStream);
	LookToRead_CreateVTable(&lookStream, False);
  
	lookStream.realStream = &archiveStream.s;
	LookToRead_Init(&lookStream);

	allocImp.Alloc = SzAlloc;
	allocImp.Free = SzFree;

	allocTempImp.Alloc = SzAllocTemp;
	allocTempImp.Free = SzFreeTemp;

	CrcGenerateTable();

	SzArEx_Init(&db);
	res = SzArEx_Open(&db, &lookStream.s, &allocImp, &allocTempImp);
	if (res != SZ_OK)
	{
		// Error opening the file.
		SzArEx_Free(&db, &allocImp);
		File_Close(&archiveStream.file);
	}
	
	file_list_t *file_list_head = NULL;
	file_list_t *file_list_end = NULL;
	
	// Go through the list of files.
	unsigned int i;
	for (i = 0; i < db.db.NumFiles; i++)
	{
		CSzFileItem *f = db.db.Files + i;
		
		// Allocate memory for the next file list element.
		file_list_t *file_list_cur = malloc(sizeof(file_list_t));
		
		// Store the ROM file information.
		file_list_cur->filename = strdup(f->Name);
		file_list_cur->filesize = f->Size;
		file_list_cur->next = NULL;
		
		// Add the ROM file information to the list.
		if (!file_list_head)
		{
			// List hasn't been created yet. Create it.
			file_list_head = file_list_cur;
			file_list_end = file_list_cur;
		}
		else
		{
			// Append the file list entry to the end of the list.
			file_list_end->next = file_list_cur;
			file_list_end = file_list_cur;
		}
	}
	
	// Close the 7z file.
	SzArEx_Free(&db, &allocImp);
	File_Close(&archiveStream.file);
	
	// Return the list of files.
	return file_list_head;
}


/**
 * decompressor_7z_get_file(): Get a file from the archive.
 * @param zF Open file handle. (Unused in the GZip handler.)
 * @param filename Filename of the archive. (Unused in the GZip handler.)
 * @param file_list Pointer to decompressor_file_list_t element to get from the archive.
 * @param buf Buffer to read the file into.
 * @param size Size of buf (in bytes).
 * @return Number of bytes read, or -1 on error.
 */
int decompressor_7z_get_file(FILE *zF, const char *filename,
			     file_list_t *file_list,
			     unsigned char *buf, const int size)
{
	return -1;
#if 0
	// All parameters (except zF) must be specified.
	if (!filename || !file_list || !buf || (size < 0))
		return -1;
	
	unzFile f = unzOpen(filename);
	if (!f)
		return -1;
	
	// Locate the ROM in the Zip file.
	if (unzLocateFile(f, file_list->filename, 1) != UNZ_OK ||
	    unzOpenCurrentFile(f) != UNZ_OK)
	{
		// Error loading the ROM file.
		// TODO: Show a message box.
		//GensUI::msgBox("Error loading the ROM file from the ZIP archive.", "ZIP File Error");
		fprintf(stderr, "%s(): Error loading the ROM file from the ZIP archive.\n", __func__);
		unzClose(f);
		return -1;
	}
	
	// Decompress the ROM.
	int zResult = unzReadCurrentFile(f, buf, size);
	unzClose(f);
	if ((zResult <= 0) || (zResult != size))
	{
		char tmp[64];
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
		
		// TODO: Show a message box.
		//GensUI::msgBox(tmp, "ZIP File Error");
		fprintf(stderr, "%s(): %s\n", __func__, tmp);
		return -1;
	}
	
	// Return the filesize.
	return size;
#endif
}
