/***************************************************************************
 * Gens: Plugin Manager Window - common functions.                         *
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "plugin_manager/plugin_manager_window.hpp"

#include <cassert>

// C++ includes
#include <string>
#include <sstream>
using std::string;
using std::stringstream;
using std::endl;

#ifdef GENS_PNG
// PNG-related variables and functions.

const unsigned char *PluginManagerWindow::png_dataptr;
unsigned int PluginManagerWindow::png_datalen;
unsigned int PluginManagerWindow::png_datapos;


/**
 * png_user_read_data(): libpng user-specified read data function.
 * Used to read PNG data from memory instead of from a file.
 * @param png_ptr Pointer to the PNG information struct.
 * @param png_bytep Pointer to memory to write PNG data to.
 * @param length Length of data requested.
 */
void PluginManagerWindow::png_user_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
	// Make sure there's enough data available.
	assert(png_datapos + length <= png_datalen);
	
	// Copy the data.
	memcpy(data, &png_dataptr[png_datapos], length);
	
	// Increment the data position.
	png_datapos += length;
}
#endif /* GENS_PNG */


/**
 * getCPUFlags(): Convert required and supported CPU flags into a string.
 * @param cpuFlagsRequired Required CPU flags.
 * @param cpuFlagsSupported Supported CPU flags.
 * @param formatting If true, uses "<b></b>" to indicate required flags; if false, uses "[flag]*".
 */
string PluginManagerWindow::GetCPUFlags(uint32_t cpuFlagsRequired,
					uint32_t cpuFlagsSupported,
					bool formatting)
{
	stringstream ssFlags;
	ssFlags << "CPU Flags: ";
	
	if (!cpuFlagsRequired && !cpuFlagsSupported)
	{
		// No CPU flags required or supported.
		ssFlags << "(none)";
		return ssFlags.str();
	}
	
	// Check all CPU flags.
	static const char* cpuFlagNames[] =
	{
		"MMX", "SSE", "SSE2", "SSE3", "SSSE3",
		"SSE4.1", "SSE4.2", "SSE4A", "SSE5",
		"MMXEXT", "3DNOW", "3DNOWEXT",
		
		NULL
	};
	
	int curFlag = 0;
	bool firstFlag = true;
	while (cpuFlagNames[curFlag])
	{
		uint32_t flagBit = (1 << curFlag);
		
		if (cpuFlagsRequired & flagBit)
		{
			// This CPU flag is required.
			if (!firstFlag)
				ssFlags << " ";
			
			if (formatting)
				ssFlags << "<b>" << cpuFlagNames[curFlag] << "</b>";
			else
				ssFlags << cpuFlagNames[curFlag] << "*";
			
			firstFlag = false;
		}
		else if (cpuFlagsSupported & flagBit)
		{
			// This CPU flag is supported, but not required.
			if (!firstFlag)
				ssFlags << " ";
			ssFlags << cpuFlagNames[curFlag];
			firstFlag = false;
		}
		
		// Next CPU flag.
		curFlag++;
	}
	
	// Return the CPU flag string.
	return ssFlags.str();
}
