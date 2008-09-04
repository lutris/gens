/***************************************************************************
 * Gens: 7z File Compression Class.                                        *
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

#include "7z.hpp"

#include <stdlib.h>
#include <string.h>

#include "emulator/g_main.hpp"
#include "ui-common.h"

// Pipe includes
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

#include <string>
#include <sstream>
using std::string;
using std::stringstream;

_7z::_7z()
{
}

_7z::~_7z()
{
}


/**
 * detectFormat(): Detect if a file is in GZip format.
 * @param f File pointer of the file to check.
 * @return True if this file is in GZip format.
 */
bool _7z::detectFormat(FILE *f)
{
	// Magic Number for 7z:
	// First six bytes: "7z\xBC\xAF\x27\x1C"
	unsigned char buf[6];
	fseek(f, 0, SEEK_SET);
	fread(buf, 6, sizeof(unsigned char), f);
	return (buf[0] == '7' && buf[1] == 'z' &&
		buf[2] == 0xBC && buf[3] == 0xAF &&
		buf[4] == 0x27 && buf[5] == 0x1C);
}


/**
 * getNumFiles(): Gets the number of files in the specified archive.
 * @param filename Filename of the archive.
 * @return Number of files, or 0 on error.
 */
int _7z::getNumFiles(string zFilename)
{
	FILE *p_7z;
	char buf[1025];
	int rv;
	stringstream ss;
	string data;
	int numFiles = 0;
	
	// Build the command line.
	// TODO: Make "/usr/bin/7z" configurable.
	string cmd = Misc_Filenames._7z_Binary;
	cmd += " l ";
	cmd += "\"" + zFilename + "\"";
	
	// Open the 7z file.
	p_7z = popen(cmd.c_str(), "r");
	if (!p_7z)
	{
		printf("Error opening p_7z: error %s.\n", strerror(errno));
		return 0;
	}
	
	// Read from the pipe.
	while ((rv = fread(buf, 1, 1024, p_7z)))
	{
		buf[rv] = 0x00;
		ss << buf;
	}
	pclose(p_7z);
	
	// Get the string and go through it to get the number of files.
	data = ss.str();
	ss.clear();
	
	// Find the ---, which indicates the start of the file listing.
	unsigned int listStart = data.find("---");
	if (listStart == string::npos)
	{
		// Not found. Either there are no files, or the archive is broken.
		return 0;
	}
	
	// Find the newline after the list start.
	unsigned int listStartLF = data.find("\n", listStart);
	if (listStart == string::npos)
	{
		// Not found. Either there are no files, or the archive is broken.
		return 0;
	}
	
	printf("listStart: %d; listStartLF: %d\n", listStart, listStartLF);
	
	// Parse all lines until we hit another "---" (or EOF).
	unsigned int curStartPos = listStartLF + 1;
	unsigned int curEndPos;
	string curLine;
	bool endOf7z = false;
	while (!endOf7z)
	{
		curEndPos = data.find("\n", curStartPos);
		if (curEndPos == string::npos)
		{
			// End of list.
			break;
		}
		
		// Get the current line.
		curLine = data.substr(curStartPos, curEndPos - curStartPos);
		
		// Check the contents of the line.
		if (curLine.substr(20, 1) == "D")
		{
			// Directory. Don't add this entry.
		}
		else if (curLine.substr(0, 3) == "---")
		{
			// End of file listing.
			endOf7z = true;
		}
		else
		{
			// File. Add this entry.
			numFiles++;
		}
		
		// Go to the next file in the listing.
		curStartPos = curEndPos + 1;
	}
	
	// Return the number of files found.
	return numFiles;
}


/**
 * getFileInfo(): Get information about all files in the specified archive.
 * @param zFilename Filename of the archive.
 * @return Pointer to list of CompressedFile structs, or NULL on error.
 */
list<CompressedFile>* _7z::getFileInfo(string zFilename)
{
	list<CompressedFile> *lst;
	CompressedFile file;
	
	FILE *p_7z;
	char buf[1025];
	int rv;
	stringstream ss;
	string data;
	int numFiles = 0;
	
	// Build the command line.
	// TODO: Make "/usr/bin/7z" configurable.
	string cmd = Misc_Filenames._7z_Binary;
	cmd += " l ";
	cmd += "\"" + zFilename + "\"";
	
	p_7z = popen(cmd.c_str(), "r");
	if (!p_7z)
	{
		printf("Error opening p_7z: error %s.\n", strerror(errno));
		return 0;
	}
	
	// Read from the pipe.
	while ((rv = fread(buf, 1, 1024, p_7z)))
	{
		buf[rv] = 0x00;
		ss << buf;
	}
	pclose(p_7z);
	
	// Get the string and go through it to get the number of files.
	data = ss.str();
	ss.clear();
	
	// Find the ---, which indicates the start of the file listing.
	unsigned int listStart = data.find("---");
	if (listStart == string::npos)
	{
		// Not found. Either there are no files, or the archive is broken.
		return 0;
	}
	
	// Find the newline after the list start.
	unsigned int listStartLF = data.find("\n", listStart);
	if (listStart == string::npos)
	{
		// Not found. Either there are no files, or the archive is broken.
		return 0;
	}
	
	printf("listStart: %d; listStartLF: %d\n", listStart, listStartLF);
	
	// Create the list.
	lst = new list<CompressedFile>;
	
	// Parse all lines until we hit another "---" (or EOF).
	unsigned int curStartPos = listStartLF + 1;
	unsigned int curEndPos;
	string curLine;
	bool endOf7z = false;
	while (!endOf7z)
	{
		curEndPos = data.find("\n", curStartPos);
		if (curEndPos == string::npos)
		{
			// End of list.
			break;
		}
		
		// Get the current line.
		curLine = data.substr(curStartPos, curEndPos - curStartPos);
		
		// Check the contents of the line.
		if (curLine.substr(20, 1) == "D")
		{
			// Directory. Don't add this entry.
		}
		else if (curLine.substr(0, 3) == "---")
		{
			// End of file listing.
			endOf7z = true;
		}
		else
		{
			// File. Add this entry.
			file.filename = curLine.substr(53);
			file.filesize = atoi(curLine.substr(26, 12).c_str());
			lst->push_back(file);
			numFiles++;
		}
		
		// Go to the next file in the listing.
		curStartPos = curEndPos + 1;
	}
	
	// Return the list of files.
	return lst;
}


/**
 * getFile(): Gets the specified file from the specified archive.
 * @param zFilename Filename of the archive.
 * @param fileInfo Information about the file to extr
 * @param buf Buffer to write the file to.
 * @param size Size of the buffer, in bytes.
 * @return Number of bytes read, or -1 on error.
 */
int _7z::getFile(string zFilename, const CompressedFile *fileInfo, unsigned char *buf, int size)
{
	FILE *p_7z;
	char buf7z[1024];
	int rv;
	stringstream ss;
	string data;
	int totalSize = 0;
	
	// Build the command line.
	// TODO: Make "/usr/bin/7z" configurable.
	string cmd = Misc_Filenames._7z_Binary;
	cmd += " e ";
	cmd += "\"" + zFilename + "\" ";
	cmd += "\"" + fileInfo->filename + "\" ";
	cmd += "-so 2>/dev/null";
	
	p_7z = popen(cmd.c_str(), "r");
	if (!p_7z)
	{
		printf("Error opening p_7z: error %s.\n", strerror(errno));
		return -1;
	}
	
	// Read from the pipe.
	while ((rv = fread(buf7z, 1, 1024, p_7z)))
	{
		if (totalSize + rv > size)
			break;
		memcpy(&buf[totalSize], &buf7z, rv);
		totalSize += rv;
	}
	pclose(p_7z);
	
	// Return the filesize.
	return totalSize;
}
