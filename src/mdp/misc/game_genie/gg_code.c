/***************************************************************************
 * [MDP] Game Genie - Code Handling Functions.                             *
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

#include "gg_code.h"

// C includes.
#include <string.h>

// Game Genie characters.
static const char gg_chars[] =
	"AaBbCcDdEeFfGgHhJjKkLlMmNnPpRrSsTtVvWwXxYyZz0O1I2233445566778899";

// Decode/Encode functions.
static int gg_code_decode_gg(const char* code, gg_code_t *gg_code);
static int gg_code_encode_gg(gg_code_t *gg_code);

static int gg_code_decode_ar(const char* code, gg_code_t *gg_code);


/**
 * gg_code_parse(): Parse a Game Genie and/or patch code.
 * @param code Original code.
 * @param gg_code gg_code_t struct to store the code.
 * @param cpu CPU this code is targetting.
 * @return 0 on success; non-zero on error.
 */
int gg_code_parse(const char* code, gg_code_t *gg_code, gg_code_cpu cpu)
{
	// Check that code and gg_code aren't NULL.
	if (!code || !gg_code)
		return 1;
	
	// Get the length of the code.
	int code_len = strlen(code);
	
	// Check if this is a Main 68K Game Genie code.
	if (cpu == CPU_M68K && code_len == 9 && code[4] == '-')
	{
		// This is potentially a Game Genie code.
		if (!gg_code_decode_gg(code, gg_code))
			return 0;
	}
	
	// Check if this is a 16-bit data code for M68K.
	// TODO: Make this more flexible.
	if (cpu == CPU_M68K && code_len == 11 && code[6] == ':')
	{
		// This is potentially an AR code.
		if (!gg_code_decode_ar(code, gg_code))
			return 0;
	}
	
	// TODO: Patch codes and other CPUs.
	return 1;
}


// Macro to get a Game Genie character.
#define GET_GENIE_CHAR(ch)			\
	if (!(x = strchr(gg_chars, code[ch])))	\
		return 1;			\
	n = ((int)(x - gg_chars)) >> 1;

/**
 * gg_code_decode_gg(): Decode a Game Genie code.
 * @param code Game Genie code.
 * @param gg_code gg_code_t to store the parsed code in.
 * @return 0 on success; non-zero on error.
 */
static int gg_code_decode_gg(const char* code, gg_code_t *gg_code)
{
	if (strlen(code) != 9 || code[4] != '-')
		return 1;
	
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
	gg_code->address  = address;
	gg_code->data     = data;
	gg_code->datasize = DS_WORD;
	gg_code->cpu      = CPU_M68K;
	
	// Re-encode the Game Genie code to ensure that the letters are correct.
	return gg_code_encode_gg(gg_code);
}


/**
 * gg_code_encode_gg(): Encode a code as Game Genie.
 * @param gg_code Pointer to gg_code_t struct with a code to encode as Game Genie.
 * @return 0 on success; non-zero on error (e.g. if the code can't be encoded as Game Genie).
 */
static int gg_code_encode_gg(gg_code_t *gg_code)
{
	// Code must be for the Main 68K and have a 16-bit data size.
	// Also, address must be <0x400000.
	if (gg_code->cpu != CPU_M68K ||
	    gg_code->datasize != DS_WORD ||
	    gg_code->address >= 0x400000)
	{
		// Cannot be encoded as a Game Genie code.
		gg_code->game_genie[0] = 0x00;
		return 1;
	}
	
	// Initialize the Game Genie code.
	gg_code->game_genie[4] = '-';
	gg_code->game_genie[9] = 0x00;
	
	// Current character.
	int ch;
	
	// Character 0: ____ ____ ____ ____ ____ ____ : ____ ____ ABCD E___
	ch = (gg_code->data >> 3) & 0x1F;
	gg_code->game_genie[0] = gg_chars[ch << 1];
	
	// Character 1: ____ ____ DE__ ____ ____ ____ : ____ ____ ____ _ABC
	ch = ((gg_code->data << 2) & 0x1C) | ((gg_code->address >> 14) & 0x03);
	gg_code->game_genie[1] = gg_chars[ch << 1];
	
	// Character 2: ____ ____ __AB CDE_ ____ ____ : ____ ____ ____ ____
	ch = (gg_code->address >> 9) & 0x1F;
	gg_code->game_genie[2] = gg_chars[ch << 1];
	
	// Character 3: BCDE ____ ____ ___A ____ ____ : ____ ____ ____ ____
	ch = ((gg_code->address >> 4) & 0x10) | ((gg_code->address >> 20) & 0x0F);
	gg_code->game_genie[3] = gg_chars[ch << 1];
	
	// Character 4: '-'
	
	// Character 5: ____ ABCD ____ ____ ____ ____ : ___E ____ ____ ____
	ch = ((gg_code->address >> 15) & 0x1E) | ((gg_code->data >> 12) & 0x01);
	gg_code->game_genie[5] = gg_chars[ch << 1];
	
	// Character 6: ____ ____ ____ ____ ____ ____ : E___ ABCD ____ ____
	ch = ((gg_code->data >> 7) & 0x1E) | ((gg_code->data >> 15) & 0x01);
	gg_code->game_genie[6] = gg_chars[ch << 1];
	
	// Character 7: ____ ____ ____ ____ CDE_ ____ : _AB_ ____ ____ ____
	ch = ((gg_code->data >> 10) & 0x18) | ((gg_code->address >> 5) & 0x07);
	gg_code->game_genie[7] = gg_chars[ch << 1];
	
	// Character 8: ____ ____ ____ ____ ___A BCDE : ____ ____ ____ ____
	ch = gg_code->address & 0x1F;
	gg_code->game_genie[8] = gg_chars[ch << 1];
	
	// Code encoded successfully.
	return 0;
}


static inline uint8_t hexToNum(char hex)
{
	if (hex >= '0' && hex <= '9')
		return (hex - '0');
	else if (hex >= 'A' && hex <= 'F')
		return ((hex - 'A') + 10);
	else if (hex >= 'a' && hex <= 'f')
		return ((hex - 'a') + 10);
	
	// Invalid hex character.
	return ~0;
}

#define AR_CHAR_DECODE(var, chr, shift)	\
{					\
	char hex = hexToNum(chr);	\
	if (hex == ~0)			\
		return 1;		\
	var |= (hex << shift);		\
}

/**
 * gg_code_decode_ar(): Decode an Action Replay code.
 * @param code Action Replay code.
 * @param gg_code gg_code_t to store the parsed code in.
 * @return 0 on success; non-zero on error.
 */
static int gg_code_decode_ar(const char* code, gg_code_t *gg_code)
{
	// TODO: Make this more flexible, so it'll accept any length code.
	if (strlen(code) != 11 || code[6] != ':')
		return 1;
	
	uint32_t address = 0;
	uint16_t data = 0;
	
	// First six characters are the address.
	AR_CHAR_DECODE(address, code[0], 20);
	AR_CHAR_DECODE(address, code[1], 16);
	AR_CHAR_DECODE(address, code[2], 12);
	AR_CHAR_DECODE(address, code[3], 8);
	AR_CHAR_DECODE(address, code[4], 4);
	AR_CHAR_DECODE(address, code[5], 0);
	
	// Last four characters are the data.
	AR_CHAR_DECODE(data, code[7], 12);
	AR_CHAR_DECODE(data, code[8], 8);
	AR_CHAR_DECODE(data, code[9], 4);
	AR_CHAR_DECODE(data, code[10], 0);
	
	// Code decoded successfully.
	gg_code->address  = address;
	gg_code->data     = data;
	gg_code->datasize = DS_WORD;
	gg_code->cpu      = CPU_M68K;
	
	// Attempt to encode as Game Genie for good measure.
	gg_code_encode_gg(gg_code);
	
	// Done.
	return 0;
}
