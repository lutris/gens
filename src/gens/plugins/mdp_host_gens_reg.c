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

#include "gens_core/cpu/68k/star_68k.h"
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


/** Register Read Functions. **/

static int mdp_host_reg_get_68k(struct S68000CONTEXT *context, int regID, uint32_t *ret_value)
{
	switch (regID)
	{
		case MDP_REG_68K_D0:
		case MDP_REG_68K_D1:
		case MDP_REG_68K_D2:
		case MDP_REG_68K_D3:
		case MDP_REG_68K_D4:
		case MDP_REG_68K_D5:
		case MDP_REG_68K_D6:
		case MDP_REG_68K_D7:
			*ret_value = context->dreg[regID - MDP_REG_68K_D0];
			break;
			
		case MDP_REG_68K_A0:
		case MDP_REG_68K_A1:
		case MDP_REG_68K_A2:
		case MDP_REG_68K_A3:
		case MDP_REG_68K_A4:
		case MDP_REG_68K_A5:
		case MDP_REG_68K_A6:
		case MDP_REG_68K_A7:
		/*case MDP_REG_68K_SP:*/
			*ret_value = context->areg[regID - MDP_REG_68K_A0];
			break;
		
		case MDP_REG_68K_ASP:
			*ret_value = context->asp;
			break;
		
		case MDP_REG_68K_PC:
			*ret_value = context->pc;
			break;
		
		case MDP_REG_68K_SR:
			*ret_value = context->sr;
			break;
		
		default:
			return -MDP_ERR_REG_INVALID_REGID;
	}
	
	return MDP_ERR_OK;
}


int MDP_FNCALL mdp_host_reg_get(int icID, int regID, uint32_t *ret_value)
{
	if (!ret_value)
		return -MDP_ERR_INVALID_PARAMETERS;
	
	switch (icID)
	{
		case MDP_REG_IC_M68K:
			return mdp_host_reg_get_68k(&main68k_context, regID, ret_value);
		default:
			return -MDP_ERR_REG_INVALID_ICID;
	}
}


int MDP_FNCALL mdp_host_reg_set(int icID, int regID, uint32_t new_value)
{
	// TODO
	return -MDP_ERR_FUNCTION_NOT_IMPLEMENTED;
}
