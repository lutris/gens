/***************************************************************************
 * Gens: MDP: Mega Drive Plugin - Host Services. (Emulator Control)        *
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

#include "mdp_host_gens_emuctrl.hpp"
#include "mdp/mdp_error.h"

// C includes.
#include <string.h>

// Byteswapping macros.
#include "libgsft/gsft_byteswap.h"

// Gens functions.
#include "emulator/options.hpp"
#include "util/file/rom.hpp"


/**
 * mdp_host_emulator_control(): Emulator control functions.
 * @param plugin Plugin requesting the function.
 * @param ctrl Control function.
 * @param param Parameter. (Contents depend on the control function.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_host_emulator_control(mdp_t *plugin, MDP_EMUCTRL ctrl, void *param)
{
	switch (ctrl)
	{
		case MDP_EMUCTRL_RESET_HARD:
			if (!Game)
			{
				// No game is running.
				// TODO: Add EMUCTRL-specific error codes.
				return -MDP_ERR_EMUCTRL_UNKNOWN;
			}
			
			// Reset the emulated system.
			Options::systemReset();
			break;
		
		case MDP_EMUCTRL_RESET_SOFT:
			// Soft reset isn't supported.
			return -MDP_ERR_EMUCTRL_FUNCTION_NOT_IMPLEMENTED;
		
		case MDP_EMUCTRL_RELOAD_INFO:
			// TODO
			return -MDP_ERR_EMUCTRL_FUNCTION_NOT_IMPLEMENTED;
		
		case MDP_EMUCTRL_UNKNOWN:
		default:
			// Invalid function.
			return -MDP_ERR_EMUCTRL_INVALID_FUNCTION;
	}
	
	return MDP_ERR_OK;
}