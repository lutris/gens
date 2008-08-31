/***************************************************************************
 * Gens: File Compression Sub Class.                                       *
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
 
#ifndef GENS_SUBCOMPRESSOR_HPP
#define GENS_SUBCOMPRESSOR_HPP

#include <stdio.h>
#include <string>
#include <list>

// Compressed file struct
#include "compressedfile.hpp"

using std::string;
using std::list;

class SubCompressor
{
	public:
		SubCompressor();
		virtual ~SubCompressor();
		
		virtual bool detectFormat(FILE *f) = 0;
		virtual int getNumFiles(string zFilename) = 0;
		virtual list<CompressedFile>* getFileInfo(string zFilename) = 0;
		virtual int getFile(string zFilename, const CompressedFile *fileInfo, unsigned char *buf, int size) = 0;
};

// Various subcompressors.
#include "gzip.hpp"

#endif
