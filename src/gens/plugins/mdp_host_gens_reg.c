/***************************************************************************
 * Gens: MDP: Mega Drive Plugin - Host Services. (Register Functions)      *
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

#include "mdp_host_gens_reg.h"
#include "mdp/mdp_error.h"
#include "mdp/mdp_reg.h"

/*
#include "gens_core/mem/mem_m68k.h"
#include "gens_core/mem/mem_s68k.h"
#include "gens_core/mem/mem_z80.h"
#include "gens_core/mem/mem_sh2.h"
#include "gens_core/vdp/vdp_io.h"
#include "gens_core/vdp/vdp_rend.h"
#include "util/file/rom.hpp"
*/

extern int ice;


int MDP_FNCALL mdp_host_reg_get(int icID, int regID, uint32_t *ret_value)
{
	// TODO
	return -MDP_ERR_FUNCTION_NOT_IMPLEMENTED;
}


int MDP_FNCALL mdp_host_reg_set(int icID, int regID, uint32_t new_value)
{
	// TODO
	return -MDP_ERR_FUNCTION_NOT_IMPLEMENTED;
}
