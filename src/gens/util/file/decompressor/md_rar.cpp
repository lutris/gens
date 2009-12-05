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

// C includes.
#include <unistd.h>
#include <stdlib.h>

// C++ includes.
#include <sstream>
using std::string;
using std::stringstream;

// MDP includes.
#include "mdp/mdp_error.h"

// libgsft includes.
#include "libgsft/gsft_strdup.h"
#include "libgsft/gsft_unused.h"


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
 * decompressor_rar_get_file_info(): Get information about all files in the archive.
 * @param zF		[in] Open file handle.
 * @param filename	[in] Filename of the archive.
 * @param z_entry_out	[out] Pointer to mdp_z_entry_t*, which will contain an allocated mdp_z_entry_t.
 * @return MDP error code.
 */
int decompressor_rar_get_file_info(FILE *zF, const char* filename, mdp_z_entry_t** z_entry_out)
{
	GSFT_UNUSED_PARAMETER(zF);
	
	if (!z_entry_out)
		return -MDP_ERR_INVALID_PARAMETERS;
	
	// Check that the RAR executable is available.
	if (access(Misc_Filenames.RAR_Binary, X_OK) != 0)
	{
		// Cannot run the RAR executable.
		return -MDP_ERR_Z_EXE_NOT_FOUND;
	}
	
	// Build the command line.
	stringstream ssCmd;
	ssCmd << "\"" << Misc_Filenames.RAR_Binary << "\" v \"" << filename << "\"";
	
	// Open the RAR file.
	FILE *pRAR = gens_popen(ssCmd.str().c_str(), "r");
	if (!pRAR)
	{
		// Error opening `rar`.
		return -MDP_ERR_Z_EXE_NOT_FOUND;
	}
	
	// Read from the pipe.
	char buf[1025];
	size_t rv;
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
		return -MDP_ERR_Z_NO_FILES_IN_ARCHIVE;
	}
	
	// Find the newline after the list start.
	unsigned int listStartLF = data.find(RAR_NEWLINE, listStart);
	if (listStart == string::npos)
	{
		// Not found. Either there are no files, or the archive is broken.
		return -MDP_ERR_Z_NO_FILES_IN_ARCHIVE;
	}
	
	// File list pointers.
	mdp_z_entry_t *z_entry_head = NULL;
	mdp_z_entry_t *z_entry_end = NULL;
	
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
			break;
		}
		
		// Get the current line.
		curLine = data.substr(curStartPos, curEndPos - curStartPos);
		
		// First line in a RAR file listing is the filename. (starting at the second character)
		if (curLine.length() < 2)
		{
			break;
		}
		if (curLine.at(0) == '-')
		{
			// End of file listing.
			break;
		}
		tmp_filename = curLine.substr(1);
		
		// Get the second line, which contains the filesize and filetype.
		curStartPos = curEndPos + RAR_NEWLINE_LENGTH;
		curEndPos = data.find(RAR_NEWLINE, curStartPos);
		if (curEndPos == string::npos)
		{
			// End of file listing.
			break;
		}
		
		// Get the current line.
		curLine = data.substr(curStartPos, curEndPos - curStartPos);
		
		// Check if this is a normal file.
		if (curLine.length() < 62)
			break;
		
		// Normal file.
		tmp_filesize = atoi(curLine.substr(12, 10).c_str());
		
		// Allocate memory for the next file list element.
		mdp_z_entry_t *z_entry_cur = (mdp_z_entry_t*)malloc(sizeof(mdp_z_entry_t));
		
		// Store the ROM file information.
		z_entry_cur->filename = strdup(tmp_filename.c_str());
		z_entry_cur->filesize = tmp_filesize;
		z_entry_cur->next = NULL;
		
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
		
		// Go to the next file in the listing.
		curStartPos = curEndPos + RAR_NEWLINE_LENGTH;
	}
	
	// If there are no files in the archive, return an error.
	if (!z_entry_head)
		return -MDP_ERR_Z_NO_FILES_IN_ARCHIVE;
	
	// Return the list of files.
	*z_entry_out = z_entry_head;
	return MDP_ERR_OK;
}


/**
 * decompressor_rar_get_file(): Get a file from the archive.
 * @param zF Open file handle. (Unused in the RAR handler.)
 * @param filename Filename of the archive.
 * @param z_entry Pointer to mdp_z_entry_t element to get from the archive.
 * @param buf Buffer to read the file into.
 * @param size Size of buf (in bytes).
 * @return Number of bytes read, or 0 on error.
 */
size_t decompressor_rar_get_file(FILE *zF, const char *filename,
				 mdp_z_entry_t *z_entry,
				 void *buf, const size_t size)
{
	GSFT_UNUSED_PARAMETER(zF);
	
	// All parameters (except zF) must be specified.
	if (!filename || !z_entry || !buf || !size)
		return 0;
	
	// Check that the RAR executable is available.
	if (access(Misc_Filenames.RAR_Binary, X_OK) != 0)
	{
		// Cannot run the RAR executable.
		// TODO: Show an error message and/or return an error code.
		return 0;
	}
	
	// Build the command line.
	stringstream ssCmd;
	ssCmd << "\"" << Misc_Filenames.RAR_Binary << "\" p -ierr \"" << filename
			<< "\" \"" << z_entry->filename << "\"";
#ifndef GENS_OS_WIN32
	ssCmd << " 2>/dev/null";
#endif
	
	// Open the RAR file.
	FILE *pRAR = gens_popen(ssCmd.str().c_str(), "r");
	if (!pRAR)
	{
		// Error opening `rar`.
		// TODO: Show an error message and/or return an error code.
		return 0;
	}
	
	// Read from the pipe.
	size_t extracted_size = 0;
	size_t rv;
	unsigned char bufRAR[1024];
	while ((rv = fread(bufRAR, 1, 1024, pRAR)))
	{
		if (extracted_size + rv > size)
			break;
		memcpy(&((unsigned char*)buf)[extracted_size], &bufRAR, rv);
		extracted_size += rv;
	}
	gens_pclose(pRAR);
	
	// Return the filesize.
	return extracted_size;
}
