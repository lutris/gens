/***************************************************************************
 * Gens: [MDP] Game Genie.                                                 *
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdint.h>
#include <string.h>

#include "mdp_misc_game_genie.hpp"
#include "mdp_misc_game_genie_plugin.h"

// MDP includes.
#include "mdp/mdp_cpuflags.h"
#include "mdp/mdp_error.h"

// MDP Host Services.
static MDP_Host_t *mdp_misc_game_genie_hostSrv = NULL;


/**
 * mdp_misc_game_genie_init(): Initialize the Game Genie plugin.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_misc_game_genie_init(MDP_Host_t *hostSrv)
{
	// Save the MDP Host Services pointer.
	mdp_misc_game_genie_hostSrv = hostSrv;
	
	// Initialized.
	return MDP_ERR_OK;
}


/**
 * mdp_misc_game_genie_end(): Shut down the Game Genie plugin.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_misc_game_genie_end(void)
{
	if (!mdp_misc_game_genie_hostSrv)
		return MDP_ERR_OK;
	
	// Plugin is shut down.
	return MDP_ERR_OK;
}
