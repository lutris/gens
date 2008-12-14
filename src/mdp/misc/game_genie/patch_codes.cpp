/***************************************************************************
 * Gens: [MDP] Game Genie - Patch Codes Handler.                           *
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

#include "patch_codes.hpp"


PatchCodes::PatchCodes()
{
	m_cpu = CPU_INVALID;
	m_dataSize = DS_INVALID;
}

PatchCodes::PatchCodes(const std::string& code)
{
	m_cpu = CPU_INVALID;
	m_dataSize = DS_INVALID;
	setCode(code, cpu);
}

PatchCodes::~PatchCodes()
{
}


/**
 * setCode(): Set the code information.
 * @param code Code string.
 * @param cpu CPU for the code.
 */
void PatchCodes::setCode(const std::string& code, CPU cpu)
{
	// Decode the code.
	
	// First, check if this is potentially an M68K Game Genie code.
	if (cpu == CPU_M68K && code.length() == 9 && code.at(4) == '-')
	{
		// Check if it's a Game Genie code.
		if (decodeGG(code))
			return;
	}
	
	// TODO
	return;
}


// Macro to get a Game Genie character.
#define GET_GENIE_CHAR(n)				\
	if (!(x = strchr(genie_chars, code.at(n))))	\
		return false;				\
	n = ((int)(x - genie_chars)) >> 1;

/**
 * decodeGG() Decode a Game Genie code.
 * @param code Game Genie code.
 * @return True if decoded successfully; false if not.
 */
bool PatchCodes::decodeGG(const std::string& code)
{
	if (code.length() != 9 || code.at(4) != '-')
		return false;
	
	// Game Genie characters.
	static const char genie_chars[] =
		"AaBbCcDdEeFfGgHhJjKkLlMmNnPpRrSsTtVvWwXxYyZz0O1I2233445566778899";
	
	// Go through all 9 characters, one at a time.
	uint32_t address = 0;
	uint16_t data = 0;
	int n; char *x;
	
	// Character 0: ____ ____ ____ ____ ____ ____ : ____ ____ ABCD E___
	GET_GENIE_CHAR(0);
	data |= (n << 3);
	
	// Character 1: ____ ____ DE__ ____ ____ ____ : ____ ____ ____ _ABC
	GET_GENIE_CHAR(1);
	data |= (n >> 2);
	address |= ((n & 3) << 14);
	
	// Character 2: ____ ____ __AB CDE_ ____ ____ : ____ ____ ____ ____
	GET_GENIE_CHAR(2);
	address |= (n << 9);
	
	// Character 3: BCDE ____ ____ ___A ____ ____ : ____ ____ ____ ____
	GET_GENIE_CHAR(3);
	address |= (((n & 0xF) << 20) | ((n >> 4) << 8));
	
	// Character 4: '-'
	
	// Character 5: ____ ABCD ____ ____ ____ ____ : ___E ____ ____ ____
	GET_GENIE_CHAR(5);
	address |= ((n >> 1) << 16);
	data |= ((n & 1) << 12);
	
	// Character 6: ____ ____ ____ ____ ____ ____ : E___ ABCD ____ ____
	GET_GENIE_CHAR(6);
	data |= (((n & 1) << 15) | ((n >> 1) << 8));
	
	// Character 7: ____ ____ ____ ____ CDE_ ____ : _AB_ ____ ____ ____
	GET_GENIE_CHAR(7);
	address |= ((n & 7) << 5);
	data |= ((n >> 3) << 13);
	
	// Character 8: ____ ____ ____ ____ ___A BCDE : ____ ____ ____ ____
	GET_GENIE_CHAR(8);
	address |= n;
	
	// Code decoded successfully.
	m_address = address;
	m_data = data;
	m_dataSize = DS_WORD;
	m_cpu = CPU_M68K;
	return true;
}
