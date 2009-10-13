/***************************************************************************
 * MDP: Game Genie. (File Handler)                                         *
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gg_file.hpp"

#include "gg.hpp"
#include "gg_code.h"

// C includes.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// C++ includes.
#include <list>
using std::list;

// libgsft includes.
#include "libgsft/gsft_strlcpy.h"
#include "libgsft/gsft_strsep.h"

// MDP includes.
#include "mdp/mdp_stdint.h"
#include "mdp/mdp_fncall.h"

// Header line for new patch code files.
static const char gg_file_header[] = "MDP Game Genie Patch Code File";


/**
 * The MDP Game Genie plugin uses a new patch code file format
 * that is incompatible with previous verisons of Gens.
 *
 * Line format:
 * CPU:Address:Data:Name
 *
 * CPU can be M68K, S68K, Z80, MSH2, or SSH2.
 * Address is the address for the patch code. (Hexadecimal, unprefixed.)
 * Data is the data for the patch code. (Hexadecimal, unprefixed.)
 * Name is the name of the code. (May contain colons.)
 */


// CPU listing.
// TODO: This is also listed somewhere else.
// Consolidate it!
static const char* const s_cpu_list[8] = {NULL, "M68K", "S68K", "Z80", "MSH2", "SSH2", NULL, NULL};

// Load an old format Game Genie patch code file.
static void MDP_FNCALL gg_file_load_old_format(FILE *f_codes);


/**
 * ELIMINATE_NEWLINES(): Eliminate newlines from a string.
 * @param str String.
 */
static inline void ELIMINATE_NEWLINES(char *str)
{
	int s_data_len;
	
	for (int i = 0; i < 2; i++)
	{
		s_data_len = strlen(str);
		if (s_data_len == 0)
			break;
		
		if (str[s_data_len - 1] == '\r' ||
		    str[s_data_len - 1] == '\n')
		{
			// Newline. Remove it.
			str[s_data_len - 1] = 0x00;
		}
		else
			break;
	}
}


/**
 * gg_file_load(): Load a Game Genie patch code file.
 * @param filename Filename of the Game Genie patch code file.
 * @return 0 on success; non-zero on error.
 */
int MDP_FNCALL gg_file_load(const char* filename)
{
	// Clear the code list.
	gg_code_list.clear();
	
	// Open the patch code file.
	FILE *f_codes = fopen(filename, "r");
	if (!f_codes)
		return 0;
	
	char in_line[256];
	
	// Check the first line to determine what format the file is in.
	fgets(in_line, sizeof(in_line), f_codes);
	if (strncmp(in_line, gg_file_header, sizeof(gg_file_header) - 1))
	{
		// Not in the new patch code file format.
		gg_file_load_old_format(f_codes);
		fclose(f_codes);
		return 1;
	}
	
	// Tokens.
	char *tokens[4];
	char *stringp;	// Used for strsep() for reentrancy.
	
	// Game Genie code.
	gg_code_t gg_code;
	int i, s_data_len;
	
	// Make sure codes are disabled initially.
	gg_code.enabled = 0;
	
	while (fgets(in_line, sizeof(in_line), f_codes))
	{
		// Tokenize the string.
		stringp = &in_line[0];
		tokens[0] = strsep(&stringp, ":");	// CPU
		tokens[1] = strsep(&stringp, ":");	// Address
		tokens[2] = strsep(&stringp, ":");	// Data
		tokens[3] = strsep(&stringp, "");	// Name (optional)
		
		// Make sure CPU, Address, and Data are not null.
		if (!tokens[0] || !tokens[1] || !tokens[2])
			continue;
		
		// Determine the CPU.
		gg_code.cpu = CPU_INVALID;
		for (i = 1; i <= 5; i++)
		{
			if (!strcmp(tokens[0], s_cpu_list[i]))
			{
				// Found the CPU.
				gg_code.cpu = (GG_CODE_CPU)(i);
			}
		}
		
		if (gg_code.cpu == CPU_INVALID)
			continue;
		
		// Get the address and data.
		gg_code.address = strtoul(tokens[1], NULL, 16);
		gg_code.data = strtoul(tokens[2], NULL, 16);
		
		// Check that the address is valid for the specified CPU.
		if (gg_code.cpu == CPU_M68K || gg_code.cpu == CPU_S68K)
		{
			// MC68000 has a 24-bit address bus.
			if (gg_code.address & 0xFF000000)
				continue;
		}
		else if (gg_code.cpu == CPU_Z80)
		{
			// Z80 has a 16-bit address bus.
			if (gg_code.address & 0xFFFF0000)
				continue;
		}
		
		// Eliminate newlines from the data token.
		ELIMINATE_NEWLINES(tokens[2]);
		
		// Determine the size of the data.
		s_data_len = strlen(tokens[2]);
		
		if (s_data_len == 0)
		{
			// No data.
			continue;
		}
		else if (s_data_len <= 2)
		{
			// 1-2 characters: 8-bit code.
			gg_code.datasize = DS_BYTE;
			gg_code.data &= 0xFF;
		}
		else if (s_data_len <= 4)
		{
			// 3-4 characters: 16-bit code.
			gg_code.datasize = DS_WORD;
			gg_code.data &= 0xFFFF;
		}
		else if (s_data_len <= 8)
		{
			// 5-8 characters: 32-bit code.
			gg_code.datasize = DS_DWORD;
		}
		else
		{
			// More than 8 characters: not valid.
			continue;
		}
		
		// Check if a name was specified.
		if (tokens[3])
		{
			// Eliminate newlines from the name token.
			ELIMINATE_NEWLINES(tokens[3]);
			
			if (strlen(tokens[3]) == 0)
			{
				// Zero-length name.
				gg_code.name[0] = 0x00;
			}
			else
			{
				// Copy the name to the gg_code.
				strlcpy(gg_code.name, tokens[3], sizeof(gg_code.name));
			}
		}
		else
		{
			// No name was specified.
			gg_code.name[0] = 0x00;
		}
		
		// Attempt to encode the code as a Game Genie code.
		gg_code_encode_gg(&gg_code);
		
		// Add the code to the list of codes.
		gg_code_list.push_back(gg_code);
	}
	
	// Close the file.
	fclose(f_codes);
	
	// Loaded successfully.
	return 0;
}


/**
 * gg_file_load_old_format(): Load an old format Game Genie patch code file.
 * @param f_codes Pointer to an open file.
 */
static void MDP_FNCALL gg_file_load_old_format(FILE *f_codes)
{
	/**
	 * Old format:
	 *
	 * Code\tName
	 *
	 * where code is the original representation of the code
	 * as entered by the user.
	 */
	
	// Seek to the beginning of the file.
	fseek(f_codes, 0, SEEK_SET);
	
	// Tokens.
	char *tokens[2];
	char *stringp;	// Used for strsep() for reentrancy.
	
	// Game Genie code.
	gg_code_t gg_code;
	
	// Make sure codes are disabled initially.
	gg_code.enabled = 0;
	
	char in_line[256];
	
	while (fgets(in_line, sizeof(in_line), f_codes))
	{
		// Tokenize the string.
		stringp = &in_line[0];
		tokens[0] = strsep(&stringp, "\t");	// Code
		tokens[1] = strsep(&stringp, "");	// Name (optional)
		
		// Make sure at least a code was specified.
		if (!tokens[0])
			continue;
		
		// Eliminate newlines from the code.
		ELIMINATE_NEWLINES(tokens[0]);
		
		// Attempt to parse the code.
		if (gg_code_parse(tokens[0], &gg_code, CPU_M68K))
		{
			// Cannot parse the code.
			continue;
		}
		
		// Check if a name was specified.
		if (tokens[1])
		{
			// Eliminate newlines from the name.
			ELIMINATE_NEWLINES(tokens[1]);
			
			// Copy the name of the code.
			strlcpy(gg_code.name, tokens[1], sizeof(gg_code.name));
		}
		else
		{
			// No name.
			gg_code.name[0] = 0x00;
		}
		
		// Add the code to the list of codes.
		gg_code_list.push_back(gg_code);
	}
}


/**
 * gg_file_save(): Save a Game Genie patch code file.
 * @param filename Filename for the Game Genie patch code file.
 * @return 0 on success; non-zero on error.
 */
int MDP_FNCALL gg_file_save(const char* filename)
{
	// NOTE: This function only uses the new Gens/GS patch code file format.
	
	// If there aren't any codes to save, don't do anything.
	if (gg_code_list.size() == 0)
		return 0;
	
	// Open the patch code file.
	FILE *f_codes = fopen(filename, "w");
	if (!f_codes)
		return 0;
	
	// Print the header line.
	fputs(gg_file_header, f_codes);
	fputs("\n", f_codes);
	
	// Buffer for the hexadecimal equivalent of the code.
	char s_code_hex[32];
	
	// Go through each code.
	for (list<gg_code_t>::iterator iter = gg_code_list.begin();
	     iter != gg_code_list.end(); iter++)
	{
		gg_code_t *gg_code = &(*iter);
		
		// Get the hex equivalent of the code.
		if (gg_code_format_hex(gg_code, s_code_hex, sizeof(s_code_hex)))
			continue;
		
		// Determine the CPU.
		const char* s_cpu = s_cpu_list[gg_code->cpu & 0x07];
		if (!s_cpu)
			continue;
		
		// Print the code string.
		fprintf(f_codes, "%s:%s", s_cpu, s_code_hex);
		if (gg_code->name[0])
			fprintf(f_codes, ":%s", gg_code->name);
		fprintf(f_codes, "\n");
	}
	
	// Close the file.
	fclose(f_codes);
	
	// Saved successfully.
	return 0;
}
