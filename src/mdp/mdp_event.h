/***************************************************************************
 * Gens: MDP: Mega Drive Plugin - Event Handler Declarations.              *
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

#ifndef __MDP_EVENT_H
#define __MDP_EVENT_H

#include <stdint.h>
#include "mdp_fncall.h"

// System ID constants.
enum MDP_SYSID
{
	MDP_SYSID_UNKNOWN = 0,
	MDP_SYSID_MD      = 1,
	MDP_SYSID_MCD     = 2,
	MDP_SYSID_32X     = 3,
	MDP_SYSID_MCD32X  = 4,
	MDP_SYSID_SMS     = 5,
	MDP_SYSID_GG      = 6,
	MDP_SYSID_SG1000  = 7,
	MDP_SYSID_PICO    = 8,
};

typedef int (MDP_FNCALL *mdp_event_load_rom_fn)(const char *rom_name, int system_id);
typedef int (MDP_FNCALL *mdp_event_close_rom_fn)(void);

#endif /* __MDP_EVENT_H */
