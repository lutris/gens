/***************************************************************************
 * MDP: Game Genie - Code Handling Functions.                              *
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
#include <stdio.h>
#include <string.h>

// libgsft includes.
#include "libgsft/gsft_szprintf.h"

// Game Genie characters.
static const char gg_chars[] =
	"AaBbCcDdEeFfGgHhJjKkLlMmNnPpRrSsTtVvWwXxYyZz0O1I2233445566778899";

// Game Genie decode function.
static int gg_code_decode_gg(const char* code, gg_code_t *gg_code);


/**
 * gg_code_parse(): Parse a Game Genie and/or patch code.
 * @param code Original code.
 * @param gg_code Pointer to gg_code_t struct to store the code.
 * @param cpu CPU this code is targetting.
 * @return GGCE_OK on success; other GG_CODE_ERR on error.
 */
GG_CODE_ERR gg_code_parse(const char* code, gg_code_t *gg_code, GG_CODE_CPU cpu)
{
	// Check that code and gg_code aren't NULL.
	if (!code || !gg_code)
		return GGCE_NULL;
	
	// Get the length of the code.
	int code_len = strlen(code);
	
	// Check if this is a Main 68K Game Genie code.
	if (cpu == CPU_M68K && code_len == 9 && code[4] == '-')
	{
		// This is potentially a Game Genie code.
		if (!gg_code_decode_gg(code, gg_code))
		{
			// Make sure the address is aligned.
			if (gg_code->address & 1)
				return GGCE_ADDRESS_ALIGNMENT;
			
			// Address is aligned.
			return GGCE_OK;
		}
	}
	
	// Check for a patch code.
	uint32_t address = 0, data = 0;
	char chrColon = 0, chrEnd = 0;
	
	// Scan the input data.
	int rval = sscanf(code, "%X%c%X%c", &address, &chrColon, &data, &chrEnd);
	
	// Make sure that at least three items were read by sscanf.
	if (rval < 3)
		return GGCE_UNRECOGNIZED;
	
	// Check that the colon character is a colon.
	if (chrColon != ':')
		return GGCE_UNRECOGNIZED;
	
	// If more than four arguments were read, make sure the fourth argument
	// is either NULL, a newline, or a carriage return.
	if (rval >= 4)
	{
		if (chrEnd != 0x00 && chrEnd != '\n' && chrEnd != '\r')
			return GGCE_UNRECOGNIZED;
	}
	
	// Check that the address is valid for the specified CPU.
	if (cpu == CPU_M68K || cpu == CPU_S68K)
	{
		// MC68000 has a 24-bit address bus.
		if (address & 0xFF000000)
			return GGCE_ADDRESS_RANGE;
	}
	else if (cpu == CPU_Z80)
	{
		// Z80 has a 16-bit address bus.
		if (address & 0xFFFF0000)
			return GGCE_ADDRESS_RANGE;
	}
	
	// Determine the data size based on the number of characters entered.
	GG_CODE_DATASIZE datasize;
	
	int pos_colon = (int)(strchr(code, ':') - code);	// Position of the ':' within the code.
	int data_chr_len = strlen(code) - pos_colon - 1;	// Length of the data segment.
	
	if (data_chr_len <= 2)
	{
		// 1-2 characters: 8-bit code.
		datasize = DS_BYTE;
		data &= 0xFF;
	}
	else if (data_chr_len <= 4)
	{
		// 3-4 characters: 16-bit code.
		datasize = DS_WORD;
		data &= 0xFFFF;
	}
	else if (data_chr_len <= 8)
	{
		// 5-8 characters: 32-bit code.
		datasize = DS_DWORD;
	}
	else
	{
		// More than 8 characters: not valid.
		return GGCE_DATA_TOO_LARGE;
	}
	
	// Check that the address is aligned properly for the specified CPU.
	if (cpu == CPU_M68K || cpu == CPU_S68K)
	{
		// 68000 requires word-alignment for 16-bit and 32-bit data.
		if ((datasize == DS_WORD || datasize == DS_DWORD) && (address & 1))
		{
			// 16-bit access, but not word-aligned.
			return GGCE_ADDRESS_ALIGNMENT;
		}
	}
#if 0
	else if (cpu == CPU_MSH2 || cpu == CPU_SSH2)
	{
		// TODO: Check the SH2's alignment restrictions.
	}
#endif
	
	// Set the members of the gg_code struct.
	gg_code->enabled  = 0;
	gg_code->address  = address;
	gg_code->data     = data;
	gg_code->cpu      = cpu;
	gg_code->datasize = datasize;
	
	// Attempt to encode the code as a Game Genie code.
	gg_code_encode_gg(gg_code);
	
	// Code parsed successfully.
	return GGCE_OK;
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
	gg_code_encode_gg(gg_code);
	
	// Decoded successfully.
	return 0;
}


/**
 * gg_code_encode_gg(): Encode a code as Game Genie.
 * @param gg_code Pointer to gg_code_t struct with a code to encode as Game Genie.
 * @return 0 on success; non-zero on error (e.g. if the code can't be encoded as Game Genie).
 */
int MDP_FNCALL gg_code_encode_gg(gg_code_t *gg_code)
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


/**
 * gg_code_format_hex(): Create the formatted hexadecimal version of a code.
 * @param gg_code Pointer to gg_code_t struct to format.
 * @param buf Buffer to write formatted code to.
 * @param size Size of the buffer.
 * @return 0 on success; non-zero on error.
 */
int gg_code_format_hex(const gg_code_t *gg_code, char *buf, size_t size)
{
	switch (gg_code->cpu)
	{
		case CPU_M68K:
		case CPU_S68K:
		{
			
			// 68000: 24-bit address.
			switch (gg_code->datasize)
			{
				case DS_BYTE:
					szprintf(buf, size, "%06X:%02X", gg_code->address, gg_code->data);
					break;
				case DS_WORD:
					szprintf(buf, size, "%06X:%04X", gg_code->address, gg_code->data);
					break;
				case DS_DWORD:
					szprintf(buf, size, "%06X:%08X", gg_code->address, gg_code->data);
					break;
				case DS_INVALID:
				default:
					// Invalid code.
					return 1;
			}
			break;
		}
		case CPU_Z80:
		{
			// Z80: 16-bit address.
			switch (gg_code->datasize)
			{
				case DS_BYTE:
					szprintf(buf, size, "%04X:%02X", gg_code->address, gg_code->data);
					break;
				case DS_WORD:
					szprintf(buf, size, "%04X:%04X", gg_code->address, gg_code->data);
					break;
				case DS_DWORD:
					szprintf(buf, size, "%04X:%08X", gg_code->address, gg_code->data);
					break;
				case DS_INVALID:
				default:
					// Invalid code.
					return 1;
			}
			break;
		}
		case CPU_MSH2:
		case CPU_SSH2:
		{
			
			// SH2: 32-bit address.
			switch (gg_code->datasize)
			{
				case DS_BYTE:
					szprintf(buf, size, "%08X:%02X", gg_code->address, gg_code->data);
					break;
				case DS_WORD:
					szprintf(buf, size, "%08X:%04X", gg_code->address, gg_code->data);
					break;
				case DS_DWORD:
					szprintf(buf, size, "%08X:%08X", gg_code->address, gg_code->data);
					break;
				case DS_INVALID:
				default:
					// Invalid code.
					return 1;
			}
			break;
		}
		case CPU_INVALID:
		default:
			// Invalid code.
			return 1;
	}
	
	// Code formatted successfully.
	return 0;
}
