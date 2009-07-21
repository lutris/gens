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

#ifndef GENS_FILE_HPP
#define GENS_FILE_HPP

#ifdef _WIN32
#define GENS_DIR_SEPARATOR_STR "\\"
#define GENS_DIR_SEPARATOR_CHR '\\'
#else /* !_WIN32 */
#define GENS_DIR_SEPARATOR_STR "/"
#define GENS_DIR_SEPARATOR_CHR '/'
#endif /* WIN32 */

#ifdef __cplusplus

#include <string>
#include <unistd.h>

class File
{
	public:
		static std::string GetNameFromPath(const std::string& fullPath);
		static std::string GetDirFromPath(const std::string& fullPath);
		
		/**
		 * exists(): Check if a file exists.
		 * @return True if the file exists; false if the file doesn't exist.
		 */
		static inline bool Exists(const std::string& filename)
		{
			// access() returns 0 on success, and -1 on failure.
			return (!access(filename.c_str(), F_OK));
		}
	
	private:
		// Don't allow instantiation of this class.
		File() { }
		~File() { }
};

#endif /* __cplusplus */

#endif /* GENS_FILE_HPP */
