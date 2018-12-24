/***************************************************************************
 * Gens: [MDP] Sonic Gens. (ROM Type Definitions)                          *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * SGens Copyright (c) 2002 by LOst                                        *
 * MDP port Copyright (c) 2008-2009 by David Korth                         *
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

#include "sgens_rom_type.h"
#include "sgens.h"

// MDP includes.
#include "mdp/mdp_constants.h"
#include "mdp/mdp.h"
#include "mdp/mdp_host.h"
#include "mdp/mdp_mem.h"

// C includes.
#include <string.h>

const char* const sgens_ROM_type_name[SGENS_ROM_TYPE_MAX + 1] =
{
	"No ROM loaded.",
	"Unsupported Mega Drive ROM.",
	"Unsupported Mega CD game.",
	"Unsupported Sega 32X ROM.",
	"Unsupported Mega CD 32X game.",
	"Unsupported Sega Master System ROM.",
	"Unsupported Game Gear ROM.",
	"Unsupported SG-1000 ROM.",
	"Unsupported Sega Pico ROM.",
	"Unsupported ROM.",
	
	"Sonic 1 (Rev. 00)",
	"Sonic 1 (Rev. 01)",
	"Sonic 1 (Rev. XB)",
	
	"Sonic 2 (Prototype; Wai Version)",
	"Sonic 2 (Rev. 00)",
	"Sonic 2 (Rev. 01)",
	"Sonic 2 (Rev. 02)",
	
	"Sonic 3",
	"Sonic & Knuckles",
	"Knuckles in Sonic 2",
	NULL
};


/**
 * sgens_get_ROM_type(): Get the ROM type of an MD ROM.
 * @param system_id System ID.
 * @return ROM type.
 */
SGENS_ROM_TYPE MDP_FNCALL sgens_get_ROM_type(int system_id)
{
	if (system_id != MDP_SYSTEM_MD)
	{
		// Only MD games are supported right now.
		switch (system_id)
		{
			case MDP_SYSTEM_MCD:
				return SGENS_ROM_TYPE_MCD_UNSUPPORTED;
			case MDP_SYSTEM_32X:
				return SGENS_ROM_TYPE_32X_UNSUPPORTED;
			case MDP_SYSTEM_MCD32X:
				return SGENS_ROM_TYPE_MCD32X_UNSUPPORTED;
			case MDP_SYSTEM_SMS:
				return SGENS_ROM_TYPE_SMS_UNSUPPORTED;
			case MDP_SYSTEM_GG:
				return SGENS_ROM_TYPE_GG_UNSUPPORTED;
			case MDP_SYSTEM_SG1000:
				return SGENS_ROM_TYPE_SG1000_UNSUPPORTED;
			case MDP_SYSTEM_PICO:
				return SGENS_ROM_TYPE_PICO_UNSUPPORTED;
			case MDP_SYSTEM_UNKNOWN:
			default:
				return SGENS_ROM_TYPE_UNSUPPORTED;
		}
	}
	
	// Determine what game this is.
	// TODO: Rewrite this to be more accurate.
	
	uint16_t tmp16;
	
	sgens_host_srv->mem_read_16(MDP_MEM_MD_ROM, 0xD354, &tmp16);
	if (tmp16 == 0x4E91)
		return SGENS_ROM_TYPE_SONIC1_REV00;
	
	sgens_host_srv->mem_read_16(MDP_MEM_MD_ROM, 0xD9E2, &tmp16);
	if (tmp16 == 0x4E91)
		return SGENS_ROM_TYPE_SONIC1_REV01;
	
	// TODO: This doesn't seem right...
	sgens_host_srv->mem_read_16(MDP_MEM_MD_ROM, 0x0000, &tmp16);
	if (tmp16 == 0x4E91)
		return SGENS_ROM_TYPE_SONIC1_REVXB;
	
	sgens_host_srv->mem_read_16(MDP_MEM_MD_ROM, 0xCFEC, &tmp16);
	if (tmp16 == 0x4E91)
		return SGENS_ROM_TYPE_SONIC2_PROTO_WAI;
	
	// TODO: Proper value for Sonic 2 rev. 00
	/*
	sgens_host_srv->mem_read_16(MDP_MEM_MD_ROM, 0x0000, &tmp16);
	if (tmp16 == 0x4E91)
		return SGENS_ROM_TYPE_SONIC2_REV00;
	*/
	
	sgens_host_srv->mem_read_16(MDP_MEM_MD_ROM, 0x15FD8, &tmp16);
	if (tmp16 == 0x4E91)
		return SGENS_ROM_TYPE_SONIC2_REV01;
	
	sgens_host_srv->mem_read_16(MDP_MEM_MD_ROM, 0x15F5E, &tmp16);
	if (tmp16 == 0x4E91)
		return SGENS_ROM_TYPE_SONIC2_REV02;
	
	sgens_host_srv->mem_read_16(MDP_MEM_MD_ROM, 0x191C6, &tmp16);
	if (tmp16 == 0x4E91)
		return SGENS_ROM_TYPE_SONIC3;
	
	sgens_host_srv->mem_read_16(MDP_MEM_MD_ROM, 0x1AB02, &tmp16);
	if (tmp16 == 0x4E91)
		return SGENS_ROM_TYPE_SONIC_KNUCKLES;
	
	// TODO: This doesn't make any sense...
	/*
	sgens_host_srv->mem_read_16(MDP_MEM_MD_ROM, 0x1AB02, &tmp16);
	if (tmp16 == 0x4E91)
		return SGENS_ROM_TYPE_SONIC2_KNUCKLES;
	*/
	
	// Unsupported MD ROM.
	return SGENS_ROM_TYPE_MD_UNSUPPORTED;
}
