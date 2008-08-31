/***************************************************************************
 * Gens: File Compression Base Class.                                      *
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

#include "compressor.hpp"

Compressor::Compressor(string zFilename)
{
	// Open the file for reading.
	m_zf = fopen(zFilename.c_str(), "rb");
	if (!m_zf)
	{
		// Error opening the file.
		m_subCompressor = NULL;
		m_fileLoaded = false;
		return;
	}
	
	// Save the filename.
	m_zFilename = zFilename;
	
	// Initialize the subCompressor.
	m_subCompressor = NULL;
	
	// Detect what type of file this is.
	SubCompressor* subCompTable[] =
	{
		new GZip(),
		NULL,
	};
	
	int i = 0;
	while (subCompTable[i])
	{
		if (subCompTable[i]->detectFormat(m_zf))
		{
			// Found the compressor.
			m_subCompressor = subCompTable[i];
			break;
		}
		i++;
	}
	
	// Delete all unused subcompressors.
	i = 0;
	while (subCompTable[i])
	{
		if (subCompTable[i] != m_subCompressor)
			delete subCompTable[i];
		i++;
	}
	
	// Close the file.
	// TODO: Keep the file open, but there's a bug in zlib where
	// it won't detect a compressed file if it's already been read.
	fclose(m_zf);
	
	// File is loaded.
	m_fileLoaded = true;
}

Compressor::~Compressor()
{
	// If a subcompressor is loaded, delete it.
	if (m_subCompressor)
		delete m_subCompressor;
}

bool Compressor::isFileLoaded(void)
{
	return m_fileLoaded;
}

int Compressor::getNumFiles(void)
{
	// Get the number of files in the archive.
	if (m_subCompressor)
		return m_subCompressor->getNumFiles(m_zFilename);
	
	// No subcompressor. Assume uncompressed.
	return 1;
}

/**
 * getFileInfo(): Get information about all files in the archive.
 * @return Pointer to list of CompressedFile structs, or NULL on error.
 */
list<CompressedFile>* Compressor::getFileInfo(void)
{
	if (m_subCompressor)
		return m_subCompressor->getFileInfo(m_zFilename);
	
	list<CompressedFile> *lst = new list<CompressedFile>;
	CompressedFile file;
	FILE *f;
	
	// Only one file is available, since this is an uncompressed file.
	
	// Set the filename.
	file.filename = m_zFilename;
	
	// Get the filesize.
	f = fopen(m_zFilename.c_str(), "rb");
	// Return NULL if the file can't be read.
	if (!f)
	{
		delete lst;
		return NULL;
	}
	
	// Get the filesize.
	fseek(f, 0, SEEK_END);
	file.filesize = ftell(f);
	fclose(f);
	
	// Add the file information to the list.
	lst->push_back(file);
	
	// Done.
	return lst;
}

/**
 * getFile(): Gets the file contents.
 * @param fileInfo Information about the file to extract.
 * @param buf Buffer to write the file to.
 * @param size Size of the buffer, in bytes.
 * @return Number of bytes read, or -1 on error.
 */
int Compressor::getFile(const CompressedFile *fileInfo, unsigned char *buf, int size)
{
	if (m_subCompressor)
		return m_subCompressor->getFile(m_zFilename, fileInfo, buf, size);
	
	int bytesRead;
	FILE *f;
	
	// All parameters (except fileInfo) must be specified.
	if (!buf || !size)
		return 0;
	
	// Open the file.
	f = fopen(m_zFilename.c_str(), "rb");
	// Return -1 if the file can't be read.
	if (!f)
		return 0;
	
	// Read the file.
	bytesRead = fread(buf, size, 1, f);
	
	// Close the file and return the number of bytes read.
	fclose(f);
	return bytesRead;
}
