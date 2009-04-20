/***************************************************************************
 * Gens: MDP: Mega Drive Plugin - Host Services. (reg_set_all() functions) *
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// C includes.
#include <stdint.h>

#include "reg_set_all.h"
#include "mdp/mdp_error.h"
#include "mdp/mdp_reg.h"

#include "util/file/rom.hpp"

#include "gens_core/cpu/68k/star_68k.h"
#include "gens_core/vdp/vdp_io.h"
#include "gens_core/sound/ym2612.hpp"
#include "gens_core/sound/psg.h"
#include "mdZ80/mdZ80.h"

extern int ice;


/**
 * mdp_host_reg_set_all(): Set all registers for the specified IC.
 * @param icID IC ID.
 * @param reg_struct Pointer to register structure for the specific IC.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_host_reg_set_all(int icID, void *reg_struct)
{
	if (!Game)
		return -MDP_ERR_ROM_NOT_LOADED;
	if (!reg_struct)
		return -MDP_ERR_INVALID_PARAMETERS;
	
	switch (icID)
	{
		case MDP_REG_IC_M68K:
		case MDP_REG_IC_VDP:
		case MDP_REG_IC_YM2612:
		case MDP_REG_IC_PSG:
		case MDP_REG_IC_Z80:
		default:
			return -MDP_ERR_REG_INVALID_ICID;
	}
}
