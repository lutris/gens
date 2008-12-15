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
#include "mdp_constants.h"

#ifdef __cplusplus
extern "C" {
#endif

// MDP Event IDs
enum MDP_EVENT_ID
{
	MDP_EVENT_UNKNOWN	= 0,
	MDP_EVENT_LOAD_ROM	= 1,
	MDP_EVENT_CLOSE_ROM	= 2
};

/**
 * MDP_EVENT_LOAD_ROM: Occurs when a ROM is loaded.
 * @param rom_name ROM name. (C string, read-only)
 * @param system_id System ID. See mdp_constants.h::MDP_SYSTEM_ID
 * @return MDP error code.
 */
typedef int (MDP_FNCALL *mdp_event_load_rom_fn)(const char *rom_name, int system_id);

/**
 * MDP_EVENT_CLOSE_ROM: Occurs when a ROM is closed.
 * @return MDP error code.
 */
typedef int (MDP_FNCALL *mdp_event_close_rom_fn)(void);

#ifdef __cplusplus
}
#endif

#endif /* __MDP_EVENT_H */
