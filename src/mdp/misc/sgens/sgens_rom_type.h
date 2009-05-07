/***************************************************************************
 * MDP: Sonic Gens. (ROM Type Definitions)                                 *
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

#ifndef _MDP_MISC_SGENS_ROM_TYPE_H
#define _MDP_MISC_SGENS_ROM_TYPE_H

#include "mdp/mdp_fncall.h"

#ifdef __cplusplus
extern "C" {
#endif

// SGens ROM definitions.
typedef enum _sgens_ROM_type
{
	SGENS_ROM_TYPE_NONE			= 0,
	SGENS_ROM_TYPE_MD_UNSUPPORTED		= 1,
	SGENS_ROM_TYPE_MCD_UNSUPPORTED		= 2,
	SGENS_ROM_TYPE_32X_UNSUPPORTED		= 3,
	SGENS_ROM_TYPE_MCD32X_UNSUPPORTED	= 4,
	SGENS_ROM_TYPE_SMS_UNSUPPORTED		= 5,
	SGENS_ROM_TYPE_GG_UNSUPPORTED		= 6,
	SGENS_ROM_TYPE_SG1000_UNSUPPORTED	= 7,
	SGENS_ROM_TYPE_PICO_UNSUPPORTED		= 8,
	SGENS_ROM_TYPE_UNSUPPORTED		= 9,
	
	SGENS_ROM_TYPE_SONIC1_REV00		= 10,
	SGENS_ROM_TYPE_SONIC1_REV01		= 11,
	SGENS_ROM_TYPE_SONIC1_REVXB		= 12,
	
	SGENS_ROM_TYPE_SONIC2_PROTO_WAI		= 13,
	SGENS_ROM_TYPE_SONIC2_REV00		= 14,
	SGENS_ROM_TYPE_SONIC2_REV01		= 15,
	SGENS_ROM_TYPE_SONIC2_REV02		= 16,
	
	SGENS_ROM_TYPE_SONIC3			= 17,
	SGENS_ROM_TYPE_SONIC_KNUCKLES		= 18,
	SGENS_ROM_TYPE_SONIC2_KNUCKLES		= 19,
	
	SGENS_ROM_TYPE_MAX			= 20
} SGENS_ROM_TYPE;

DLL_LOCAL extern const char* const sgens_ROM_type_name[SGENS_ROM_TYPE_MAX + 1];

DLL_LOCAL SGENS_ROM_TYPE MDP_FNCALL sgens_get_ROM_type(int system_id);

#ifdef __cplusplus
}
#endif

#endif /* _MDP_MISC_SGENS_ROM_TYPE_H */
