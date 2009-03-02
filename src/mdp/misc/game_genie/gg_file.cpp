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

#include "gg_file.hpp"

#include "gg.hpp"
#include "gg_code.h"

// C includes.
#include <stdio.h>

// C++ includes.
#include <list>
using std::list;

// MDP includes.
#include "mdp/mdp_fncall.h"


/**
 * gg_file_load(): Load a Game Genie patch code file.
 * @param filename Filename of the Game Genie patch code file.
 * @return 0 on success; non-zero on error.
 */
int MDP_FNCALL gg_file_load(const char* filename)
{
	// TODO
}


/**
 * gg_file_save(): Save a Game Genie patch code file.
 * @param filename Filename for the Game Genie patch code file.
 * @return 0 on success; non-zero on error.
 */
int MDP_FNCALL gg_file_save(const char* filename)
{
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
	
	// If there aren't any codes to save, don't do anything.
	if (gg_code_list.size() == 0)
		return 0;
	
	// Open the patch code file.
	FILE *f_codes = fopen(filename, "w");
	if (!f_codes)
		return 0;
	
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
		// TODO: Move this array somewhere else.
		static const char* const s_cpu_list[8] = {NULL, "M68K", "S68K", "Z80", "MSH2", "SSH2", NULL, NULL};
		
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
}
