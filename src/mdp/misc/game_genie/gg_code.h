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

#ifndef __MDP_MISC_GAME_GENIE_CODE_H
#define __MDP_MISC_GAME_GENIE_CODE_H

#include <stdint.h>

#include "mdp/mdp_fncall.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * gg_code_cpu: CPU type for a code.
 */
typedef enum _gg_code_cpu
{
	CPU_INVALID = 0,
	CPU_M68K    = 1,
	CPU_S68K    = 2,
	CPU_Z80     = 3,
	CPU_MSH2    = 4,
	CPU_SSH2    = 5,
} gg_code_cpu;

/**
 * gg_code_datasize: Data size of a code.
 */
typedef enum _gg_code_datasize
{
	DS_INVALID = 0,
	DS_BYTE    = 1, // 8-bit
	DS_WORD    = 2, // 16-bit
	DS_DWORD   = 3, // 32-bit
} gg_code_datasize;

/**
 * gg_code_t: Contains a Game Genie and/or patch code.
 */
typedef struct _gg_code_t
{
	int		enabled;
	uint32_t	address;
	uint32_t	data;
	
	gg_code_datasize	datasize;
	gg_code_cpu		cpu;
	
	// Backup area for ROM-modifying codes.
	uint32_t	backup_data;
	
	// Game Genie code.
	char		game_genie[16];
	
	// Code name.
	char		name[128];
} gg_code_t;

/**
 * gg_code_parse(): Parse a Game Genie and/or patch code.
 * @param code Original code.
 * @param gg_code gg_code_t struct to store the code.
 * @param cpu CPU this code is targetting.
 * @return 0 on success; non-zero on error.
 */
DLL_LOCAL int gg_code_parse(const char* code, gg_code_t *gg_code, gg_code_cpu cpu);

#ifdef __cplusplus
}
#endif

#endif /* __MDP_MISC_GAME_GENIE_CODE_H */
