/***************************************************************************
 * Gens: RAR Decompressor.                                                 *
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

#include "md_rar.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "emulator/g_main.hpp"
#include "ui/gens_ui.hpp"

// popen wrapper
#include "popen_wrapper.h"

// Newline constant: "\r\n" on Win32, "\n" on everything else.
#ifdef GENS_OS_WIN32
#define RAR_NEWLINE "\r\n"
#define RAR_NEWLINE_LENGTH 2
#define RAR_NAME "WinRAR"
#else
#define RAR_NEWLINE "\n"
#define RAR_NEWLINE_LENGTH 1
#define RAR_NAME "rar"
#endif

// C++ includes.
#include <sstream>
using std::string;
using std::stringstream;


/**
 * decompressor_rar_detect_format(): Detect if this file can be handled by this decompressor.
 * @param zF Open file handle.
 * @return Non-zero if this file can be handled; 0 if it can't be.
 */
int decompressor_rar_detect_format(FILE *zF)
{
	// Magic Number for RAR:
	// First four bytes: "Rar!"
	static const unsigned char rar_magic[] = {'R', 'a', 'r', '!'};
	
	unsigned char buf[sizeof(rar_magic)];
	fseek(zF, 0, SEEK_SET);
	fread(buf, sizeof(unsigned char), sizeof(rar_magic), zF);
	return (memcmp(buf, rar_magic, sizeof(rar_magic)) == 0);
}


/**
 * decompressor_rar_get_file_info(): Get information from all files in the archive.
 * @param zF Open file handle.
 * @param filename Filename of the archive.
 * @return Pointer to the first file in the list, or NULL on error.
 */
file_list_t* decompressor_rar_get_file_info(FILE *zF, const char* filename)
{
	// Unused parameters.
	((void)zF);
	
	// Build the command line.
	stringstream ssCmd;
	ssCmd << "\"" << Misc_Filenames.RAR_Binary << "\" v \"" << filename << "\"";
	
	// Open the RAR file.
	FILE *pRAR = gens_popen(ssCmd.str().c_str(), "r");
	if (!pRAR)
	{
		// Error opening RAR.
		// TODO: Show an error message.
		printf("ERR1\n");
		return NULL;
	}
	
	// Read from the pipe.
	char buf[1025];
	int rv;
	stringstream ss;
	while ((rv = fread(buf, 1, 1024, pRAR)))
	{
		buf[rv] = 0x00;
		ss << buf;
	}
	gens_pclose(pRAR);
	
	// Get the string and go through it to get the file listing.
	string data = ss.str();
	ss.clear();
	
	// Find the "---", which indicates the start of the file listing.
	unsigned int listStart = data.find("---");
	if (listStart == string::npos)
	{
		// Not found. Either there are no files, or the archive is broken.
		printf("ERR2\n");
		return NULL;
	}
	
	// Find the newline after the list start.
	unsigned int listStartLF = data.find(RAR_NEWLINE, listStart);
	if (listStart == string::npos)
	{
		// Not found. Either there are no files, or the archive is broken.
		printf("ERR3\n");
		return NULL;
	}
	
	// File list pointers.
	file_list_t *file_list_head = NULL;
	file_list_t *file_list_end = NULL;
	
	// Parse all lines until we hit another "---" (or EOF).
	unsigned int curStartPos = listStartLF + RAR_NEWLINE_LENGTH;
	unsigned int curEndPos;
	string curLine;
	bool endOfRAR = false;
	
	// Temporary data.
	string tmp_filename;
	size_t tmp_filesize;
	
	while (!endOfRAR)
	{
		curEndPos = data.find(RAR_NEWLINE, curStartPos);
		if (curEndPos == string::npos)
		{
			// End of file listing.
			printf("ERR4\n");
			break;
		}
		
		// Get the current line.
		curLine = data.substr(curStartPos, curEndPos - curStartPos);
		
		// First line in a RAR file listing is the filename. (starting at the second character)
		if (curLine.length() < 2)
		{
			printf("ERR5\n");
			break;
		}
		if (curLine.at(0) == '-')
		{
			// End of file listing.
			printf("ERR6\n");
			break;
		}
		tmp_filename = curLine.substr(1);
		
		// Get the second line, which contains the filesize and filetype.
		curStartPos = curEndPos + RAR_NEWLINE_LENGTH;
		curEndPos = data.find(RAR_NEWLINE, curStartPos);
		if (curEndPos == string::npos)
		{
			// End of file listing.
			printf("ERR7\n");
			break;
		}
		
		// Get the current line.
		curLine = data.substr(curStartPos, curEndPos - curStartPos);
		
		// Check if this is a normal file.
		if (curLine.length() < 62)
		{
			printf("ERR8\n");
			break;
		}
		
		// Normal file.
		tmp_filesize = atoi(curLine.substr(12, 10).c_str());
		printf("NORMAL: %s %d\n", tmp_filename.c_str(), tmp_filesize);
		
		// Allocate memory for the next file list element.
		file_list_t *file_list_cur = (file_list_t*)malloc(sizeof(file_list_t));
		
		// Store the ROM file information.
		file_list_cur->filename = strdup(tmp_filename.c_str());
		file_list_cur->filesize = tmp_filesize;
		file_list_cur->next = NULL;
		
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
		
		// Go to the next file in the listing.
		curStartPos = curEndPos + RAR_NEWLINE_LENGTH;
	}
	
	// Return the list of files.
	return file_list_head;
}


/**
 * decompressor_rar_get_file(): Get a file from the archive.
 * @param zF Open file handle. (Unused in the RAR handler.)
 * @param filename Filename of the archive.
 * @param file_list Pointer to decompressor_file_list_t element to get from the archive.
 * @param buf Buffer to read the file into.
 * @param size Size of buf (in bytes).
 * @return Number of bytes read, or 0 on error.
 */
size_t decompressor_rar_get_file(FILE *zF, const char *filename,
				 file_list_t *file_list,
				 void *buf, const size_t size)
{
#if 0
	// All parameters (except zF) must be specified.
	if (!filename || !file_list || !buf || !size)
		return 0;
	
	unzFile f = unzOpen(filename);
	if (!f)
		return 0;
	
	// Locate the ROM in the RAR file.
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
