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

#include "gens_core/vdp/vdp_io.h"

// Gens functions.
#include "emulator/options.hpp"
#include "emulator/g_md.hpp"
#include "util/file/rom.hpp"
#include "util/file/save.hpp"

static int mdp_host_emulator_control_reload_info(void) throw();


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
			return mdp_host_emulator_control_reload_info();
		
		case MDP_EMUCTRL_UNKNOWN:
		default:
			// Invalid function.
			return -MDP_ERR_EMUCTRL_INVALID_FUNCTION;
	}
	
	return MDP_ERR_OK;
}


/**
 * mdp_host_emulator_control_reload_info(): Reload ROM information.
 * @return MDP error code.
 */
static int mdp_host_emulator_control_reload_info(void) throw()
{
	// Save SRAM or BRAM, depending on what's loaded.
	if (!SegaCD_Started)
		Savestate::SaveSRAM();
	else
		Savestate::SaveBRAM();
	
	// Reload the ROM header.
	ROM::fillROMInfo(Game);
	
	// Reload SRAM or BRAM, depending on what's loaded.
	if (!SegaCD_Started)
	{
		Init_Genesis_SRAM(Game);
		Savestate::LoadSRAM();
	}
	else
		Savestate::LoadBRAM();
	
	// Reinitialize the title bar.
	if (Genesis_Started)
		Options::setGameName(0);
	else if (SegaCD_Started)
		Options::setGameName(1);
	else if (_32X_Started)
		Options::setGameName(2);
	
	// Done.
	return MDP_ERR_OK;
}
