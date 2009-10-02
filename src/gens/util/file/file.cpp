/***************************************************************************
 * Gens: File management functions.                                        *
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

#include "file.hpp"

// C includes
#include <unistd.h>

// C++ includes
#include <string>
using std::string;

// libgsft includes.
#include "libgsft/gsft_file.h"


/**
 * GetNameFromPath(): Get the filename part of a pathname.
 * @param fullPath Full pathname.
 * @return Filename part of the pathname.
 */
string File::GetNameFromPath(const string& fullPath)
{
	size_t pos = fullPath.rfind(GSFT_DIR_SEP_CHR);
	
	if (pos == string::npos)
		return fullPath;
	else if (pos + 1 == fullPath.length())
		return "";
	
	return fullPath.substr(pos + 1);
}


/**
 * GetDirFromPath(): Get the directory part of a pathname.
 * @param fullPath Full pathname.
 * @return Directory part of the pathname.
 */
string File::GetDirFromPath(const string& fullPath)
{
	size_t pos = fullPath.rfind(GSFT_DIR_SEP_CHR);
	
	if (pos == string::npos)
		return "";
	
	return fullPath.substr(0, pos + 1);
}
