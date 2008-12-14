/***************************************************************************
 * Gens: [MDP] Game Genie - Patch Code Handler.                            *
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

#include "patch_code.hpp"

#include <cstring>
#include <string>
using std::string;


// Game Genie characters.
const char PatchCode::gg_chars[] =
	"AaBbCcDdEeFfGgHhJjKkLlMmNnPpRrSsTtVvWwXxYyZz0O1I2233445566778899";

PatchCode::PatchCode()
{
	m_cpu = CPU_INVALID;
	m_dataSize = DS_INVALID;
}

PatchCode::PatchCode(const std::string& code, CPU cpu)
{
	m_cpu = CPU_INVALID;
	m_dataSize = DS_INVALID;
	setCode(code, cpu);
}

PatchCode::~PatchCode()
{
}


/**
 * setCode(): Set the code information.
 * @param code Code string.
 * @param cpu CPU for the code.
 */
void PatchCode::setCode(const std::string& code, CPU cpu)
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
#define GET_GENIE_CHAR(ch)				\
	if (!(x = strchr(gg_chars, code.at(ch))))	\
		return false;				\
	n = ((int)(x - gg_chars)) >> 1;

/**
 * decodeGG(): Decode a Game Genie code.
 * @param code Game Genie code.
 * @return True if decoded successfully; false if not.
 */
bool PatchCode::decodeGG(const std::string& code)
{
	if (code.length() != 9 || code.at(4) != '-')
		return false;
	
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


/**
 * getGG(): Get the current code in Game Genie format.
 * @return Game Genie code if valid; otherwise, empty string.
 */
string PatchCode::getGG(void)
{
	// Code must be for M68K and have a 16-bit data size.
	if (m_cpu != CPU_M68K || m_dataSize != DS_WORD)
		return "";
	
	// Temporary storage for the Game Genie code.
	char code[9];
	code[4] = '-';
	code[9] = 0x00;
	
	// Current character.
	int ch;
	
	// Character 0: ____ ____ ____ ____ ____ ____ : ____ ____ ABCD E___
	ch = (m_data >> 3) & 0x1F;
	code[0] = gg_chars[ch << 1];
	
	// Character 1: ____ ____ DE__ ____ ____ ____ : ____ ____ ____ _ABC
	ch = ((m_data << 2) & 0x1C) | ((m_address >> 14) & 0x03);
	code[1] = gg_chars[ch << 1];
	
	// Character 2: ____ ____ __AB CDE_ ____ ____ : ____ ____ ____ ____
	ch = (m_address >> 9) & 0x1F;
	code[2] = gg_chars[ch << 1];
	
	// Character 3: BCDE ____ ____ ___A ____ ____ : ____ ____ ____ ____
	ch = ((m_address >> 4) & 0x10) | ((m_address >> 20) & 0x0F);
	code[3] = gg_chars[ch << 1];
	
	// Character 4: '-'
	
	// Character 5: ____ ABCD ____ ____ ____ ____ : ___E ____ ____ ____
	ch = ((m_address >> 15) & 0x1E) | ((m_data >> 12) & 0x01);
	code[5] = gg_chars[ch << 1];
	
	// Character 6: ____ ____ ____ ____ ____ ____ : E___ ABCD ____ ____
	ch = ((m_data >> 7) & 0x1E) | ((m_data >> 15) & 0x01);
	code[6] = gg_chars[ch << 1];
	
	// Character 7: ____ ____ ____ ____ CDE_ ____ : _AB_ ____ ____ ____
	ch = ((m_data >> 10) & 0x18) | ((m_address >> 5) & 0x07);
	code[7] = gg_chars[ch << 1];
	
	// Character 8: ____ ____ ____ ____ ___A BCDE : ____ ____ ____ ____
	ch = m_address & 0x1F;
	code[8] = gg_chars[ch << 1];
	
	// Code encoded successfully.
	return string(code);
}
