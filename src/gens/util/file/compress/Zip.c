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
 * Zip_Get_First_File_Size(): Gets the filesize of the first file in the specified archive.
 * @param filename Filename of the archive.
 * @return Filesize, or 0 on error.
 */
int Zip_Get_First_File_Size(const char *filename)
{
	unzFile f;
	unz_file_info zinfo;
	int i, extpos;
	int filesize = 0;
	char ROMFileName[132];
	
	f = unzOpen(filename);
	if (!f)
		return 0;
	
	// TODO: Figure out some way to optimize this so the program doesn't have to
	// search through the Zip archive a second time to find the ROM file.
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
			filesize = zinfo.uncompressed_size;
			break;
		}
		
		i = unzGoToNextFile(f);
	}
	unzClose(f);
	
	if ((i != UNZ_END_OF_LIST_OF_FILE && i != UNZ_OK) || filesize == 0)
	{
		UI_MsgBox("No Genesis or 32X ROM file found in ZIP archive.", "ZIP File Error");
		return 0;
	}
	
	// Return the filesize.
	return filesize;
}

/**
 * Zip_Get_First_File(): Gets the first file from the specified archive.
 * @param filename Filename of the archive.
 * @param buf Buffer to write the file to.
 * @param size Size of the buffer, in bytes.
 * @return Number of bytes read, or -1 on error.
 */
int Zip_Get_First_File(const char *filename, void *buf, int size)
{
	unzFile f;
	unz_file_info zinfo;
	int i, extpos, zResult;
	int filesize = 0;
	char ROMFileName[132];
	char tmp[64];
	
	f = unzOpen(filename);
	if (!f)
		return 0;
	
	// TODO: Figure out some way to optimize this so the program doesn't have to
	// search through the Zip archive a second time to find the ROM file.
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
			filesize = zinfo.uncompressed_size;
			break;
		}
		
		i = unzGoToNextFile(f);
	}
	
	if ((i != UNZ_END_OF_LIST_OF_FILE && i != UNZ_OK) || filesize == 0)
	{
		// Error finding the ROM.
		unzClose(f);
		return -1;
	}

	// TODO: The above code needs to be consolidated somehow.
	
	// Locate the ROM in the Zip file.
	if (unzLocateFile(f, ROMFileName, 1) != UNZ_OK ||
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
