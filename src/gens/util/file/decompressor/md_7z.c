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

#include "ui/gens_ui.hpp"

// C includes.
#include <stdlib.h>

// 7z includes.
#include "lzma/7zCrc.h"
#include "lzma/7zFile.h"
#include "lzma/7zVersion.h"

#include "lzma/Archive/7z/7zAlloc.h"
#include "lzma/Archive/7z/7zExtract.h"
#include "lzma/Archive/7z/7zIn.h"

// MDP includes.
#include "mdp/mdp_error.h"

// libgsft includes.
#include "libgsft/gsft_strdup.h"
#include "libgsft/gsft_unused.h"


// 7z decompressor functions.
static int decompressor_7z_detect_format(FILE *zF);
static int decompressor_7z_get_file_info(FILE *zF, const char* filename,
					 mdp_z_entry_t** z_entry_out);
static size_t decompressor_7z_get_file(FILE *zF, const char* filename,
				       mdp_z_entry_t *z_entry,
				       void *buf, const size_t size);

// 7z decompressor struct.
const decompressor_t decompressor_7z =
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
 * decompressor_7z_get_file_info(): Get information about all files in the archive.
 * @param zF		[in] Open file handle.
 * @param filename	[in] Filename of the archive.
 * @param z_entry_out	[out] Pointer to mdp_z_entry_t*, which will contain an allocated mdp_z_entry_t.
 * @return MDP error code.
 */
static int decompressor_7z_get_file_info(FILE *zF, const char* filename, mdp_z_entry_t** z_entry_out)
{
	GSFT_UNUSED_PARAMETER(zF);
	
	if (!z_entry_out)
		return -MDP_ERR_INVALID_PARAMETERS;
	
	CFileInStream archiveStream;
	CLookToRead lookStream;
	CSzArEx db;
	SRes res;
	ISzAlloc allocImp;
	ISzAlloc allocTempImp;
	
	// Open the 7z file.
	if (InFile_Open(&archiveStream.file, filename))
		return -MDP_ERR_Z_ARCHIVE_NOT_FOUND;
	
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
		return -MDP_ERR_Z_CANT_OPEN_ARCHIVE;
	}
	
	mdp_z_entry_t *z_entry_head = NULL;
	mdp_z_entry_t *z_entry_end = NULL;
	
	// Go through the list of files.
	unsigned int i;
	for (i = 0; i < db.db.NumFiles; i++)
	{
		CSzFileItem *f = db.db.Files + i;
		if (f->IsDir)
			continue;
		
		// Allocate memory for the next file list element.
		mdp_z_entry_t *z_entry_cur = (mdp_z_entry_t*)malloc(sizeof(mdp_z_entry_t));
		
		// Store the ROM file information.
		z_entry_cur->filename = (f->Name ? strdup(f->Name) : NULL);
		z_entry_cur->filesize = f->Size;
		z_entry_cur->next = NULL;
		
		// Add the ROM file information to the list.
		if (!z_entry_head)
		{
			// List hasn't been created yet. Create it.
			z_entry_head = z_entry_cur;
			z_entry_end = z_entry_cur;
		}
		else
		{
			// Append the file list entry to the end of the list.
			z_entry_end->next = z_entry_cur;
			z_entry_end = z_entry_cur;
		}
	}
	
	// Close the 7z file.
	SzArEx_Free(&db, &allocImp);
	File_Close(&archiveStream.file);
	
	// If there are no files in the archive, return an error.
	if (!z_entry_head)
		return -MDP_ERR_Z_NO_FILES_IN_ARCHIVE;
	
	// Return the list of files.
	*z_entry_out = z_entry_head;
	return MDP_ERR_OK;
}


/**
 * decompressor_7z_get_file(): Get a file from the archive.
 * @param zF Open file handle. (Unused in the GZip handler.)
 * @param filename Filename of the archive. (Unused in the GZip handler.)
 * @param z_entry Pointer to mdp_z_entry_t element to get from the archive.
 * @param buf Buffer to read the file into.
 * @param size Size of buf (in bytes).
 * @return Number of bytes read, or 0 on error.
 */
static size_t decompressor_7z_get_file(FILE *zF, const char *filename,
				       mdp_z_entry_t *z_entry,
				       void *buf, const size_t size)
{
	GSFT_UNUSED_PARAMETER(zF);
	
	// All parameters (except zF) must be specified.
	if (!filename || !z_entry || !buf || !size)
		return -1;
	
	CFileInStream archiveStream;
	CLookToRead lookStream;
	CSzArEx db;
	SRes res;
	ISzAlloc allocImp;
	ISzAlloc allocTempImp;
	
	// Open the 7z file.
	if (InFile_Open(&archiveStream.file, filename))
		return 0;
	
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
		return 0;
	}
	
	UInt32 blockIndex = 0xFFFFFFFF;	/* it can have any value before first call (if outBuffer = 0) */
	Byte *outBuffer = NULL;		/* it must be 0 before first call for each new archive. */
	size_t outBufferSize = 0; 	/* it can have any value before first call (if outBuffer = 0) */
	
	// Extract the specified file.
	unsigned int i;
	const unsigned int numFiles = db.db.NumFiles;
	size_t extracted_size = 0;
	
	for (i = 0; i < numFiles; i++)
	{
		size_t offset;
		size_t outSizeProcessed;
		
		CSzFileItem *f = db.db.Files + i;
		if (f->IsDir)
			continue;
		
#ifdef GENS_OS_WIN32
		if (strcasecmp(z_entry->filename, f->Name) != 0)
#else /* !GENS_OS_WIN32 */
		if (strcmp(z_entry->filename, f->Name) != 0)
#endif /* GENS_OS_WIN32 */
		{
			// Not the correct file.
			continue;
		}
		
		// Found the file.
		res = SzAr_Extract(&db, &lookStream.s, i,
				   &blockIndex, &outBuffer, &outBufferSize,
				   &offset, &outSizeProcessed,
				   &allocImp, &allocTempImp);
		
		if (res != SZ_OK)
		{
			// Error extracting.
			break;
		}
		
		// Copy the 7z buffer to the output buffer.
		extracted_size = (size < outSizeProcessed ? size : outSizeProcessed);
		memcpy(buf, outBuffer + offset, extracted_size);
		
		// Free the 7z buffer.
		IAlloc_Free(&allocImp, outBuffer);
		
		// ROM processed.
		break;
	}
	
	// Close the 7z file.
	SzArEx_Free(&db, &allocImp);
	File_Close(&archiveStream.file);
	
	if (i >= numFiles)
		return 0;
	
	return extracted_size;
}
