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

#ifndef __MDP_MISC_GAME_GENIE_CODE_H
#define __MDP_MISC_GAME_GENIE_CODE_H

#include "mdp/mdp_stdint.h"
#include "mdp/mdp_fncall.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * GG_CODE_CPU: CPU type for a code.
 */
typedef enum _GG_CODE_CPU
{
	CPU_INVALID = 0,
	CPU_M68K    = 1,
	CPU_S68K    = 2,
	CPU_Z80     = 3,
	CPU_MSH2    = 4,
	CPU_SSH2    = 5
} GG_CODE_CPU;

/**
 * gg_code_datasize: Data size of a code.
 */
typedef enum _GG_CODE_DATASIZE
{
	DS_INVALID = 0,
	DS_BYTE    = 1, // 8-bit
	DS_WORD    = 2, // 16-bit
	DS_DWORD   = 3  // 32-bit
} GG_CODE_DATASIZE;

/**
 * gg_code_t: Contains a Game Genie and/or patch code.
 */
typedef struct _gg_code_t
{
	int		enabled;
	uint32_t	address;
	uint32_t	data;
	
	GG_CODE_CPU		cpu;
	GG_CODE_DATASIZE	datasize;
	
	// Backup area for ROM-modifying codes.
	uint32_t	backup_data;
	
	// Game Genie code.
	char		game_genie[16];
	
	// Code name.
	char		name[128];
} gg_code_t;

/**
 * GG_CODE_ERR: Error codes from gg_code_parse().
 */
typedef enum _GG_CODE_ERR
{
	GGCE_OK			= 0, // Decoded successfully.
	GGCE_NULL		= 1, // NULL pointers were passed.
	GGCE_UNRECOGNIZED	= 2, // Unrecognized code.
	GGCE_ADDRESS_RANGE	= 3, // Address out of range.
	GGCE_ADDRESS_ALIGNMENT	= 4, // Adddress alignment is incorrect for the datasize.
	GGCE_DATA_TOO_LARGE	= 5  // Data is too large.
} GG_CODE_ERR;

/**
 * gg_code_parse(): Parse a Game Genie and/or patch code.
 * @param code Original code.
 * @param gg_code Pointer to gg_code_t struct to store the code.
 * @param cpu CPU this code is targetting.
 * @return GGCE_OK on success; other gg_code_parse_err on error.
 */
DLL_LOCAL GG_CODE_ERR gg_code_parse(const char* code, gg_code_t *gg_code, GG_CODE_CPU cpu);

/**
 * gg_code_encode_gg(): Encode a code as Game Genie.
 * @param gg_code Pointer to gg_code_t struct with a code to encode as Game Genie.
 * @return 0 on success; non-zero on error (e.g. if the code can't be encoded as Game Genie).
 */
DLL_LOCAL int MDP_FNCALL gg_code_encode_gg(gg_code_t *gg_code);

/**
 * gg_code_format_hex(): Create the formatted hexadecimal version of a code.
 * @param gg_code Pointer to gg_code_t struct to format.
 * @param buf Buffer to write formatted code to.
 * @param size Size of the buffer.
 * @return 0 on success; non-zero on error.
 */
DLL_LOCAL int gg_code_format_hex(const gg_code_t *gg_code, char *buf, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* __MDP_MISC_GAME_GENIE_CODE_H */
