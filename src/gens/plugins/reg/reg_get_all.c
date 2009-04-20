/***************************************************************************
 * Gens: MDP: Mega Drive Plugin - Host Services. (reg_get_all() functions) *
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

#include "reg_get_all.h"
#include "mdp/mdp_error.h"
#include "mdp/mdp_reg.h"

#include "util/file/rom.hpp"

#include "gens_core/cpu/68k/star_68k.h"
#include "gens_core/vdp/vdp_io.h"
#include "gens_core/sound/ym2612.hpp"
#include "gens_core/sound/psg.h"
#include "mdZ80/mdZ80.h"


/**
 * mdp_host_reg_get_all_68k(): Get all 68000 registers.
 * @param context 68000 context.
 * @param reg_struct Pointer to mdp_reg_68k_t struct to store the registers in.
 * @return MDP error code.
 */
static int mdp_host_reg_get_all_68k(struct S68000CONTEXT *context, void *reg_struct)
{
	mdp_reg_68k_t *reg_68k = (mdp_reg_68k_t*)reg_struct;
	int i;
	
	for (i = 0; i < 8; i++)
	{
		reg_68k->dreg[i] = context->dreg[i];
		reg_68k->areg[i] = context->areg[i];
	}
	
	reg_68k->asp = context->asp;
	reg_68k->pc = context->pc;
	reg_68k->sr = context->sr;
	
	return MDP_ERR_OK;
}


/**
 * mdp_host_reg_get_all_vdp(): Get all VDP registers.
 * @param reg_struct Pointer to mdp_reg_vdp_t struct to store the registers in.
 * @return MDP error code.
 */
static int mdp_host_reg_get_all_vdp(void *reg_struct)
{
	mdp_reg_vdp_t *reg_vdp = (mdp_reg_vdp_t*)reg_struct;
	
	// VDP_Reg is an array of 24 unsigned ints.
	unsigned int *vdp_reg_int = (unsigned int*)(&VDP_Reg);
	int i;
	for (i = 0; i < 24; i++)
	{
		reg_vdp->data[i] = vdp_reg_int[i] & 0xFF;
	}
	
	return MDP_ERR_OK;
}


/**
 * mdp_host_reg_get_all_ym2612(): Get all YM2612 registers.
 * @param reg_struct Pointer to mdp_reg_ym2612_t struct to store the registers in.
 * @return MDP error code.
 */
static int mdp_host_reg_get_all_ym2612(void *reg_struct)
{
	mdp_reg_ym2612_t *reg_ym2612 = (mdp_reg_ym2612_t*)reg_struct;
	
	int i;
	for (i = 0; i < 0x100; i++)
	{
		reg_ym2612->regs[0][i] = YM2612_Get_Reg(i);
		reg_ym2612->regs[1][i] = YM2612_Get_Reg(0x100 | i);
	}
	
	return MDP_ERR_OK;
}


/**
 * mdp_host_reg_get_all_psg(): Get all PSG registers.
 * @param reg_struct Pointer to mdp_reg_psg_t struct to store the registers in.
 * @return MDP error code.
 */
static int mdp_host_reg_get_all_psg(void *reg_struct)
{
	mdp_reg_psg_t *reg_psg = (mdp_reg_psg_t*)reg_struct;
	
	int i;
	for (i = 0; i < 8; i++)
	{
		reg_psg->regs[i] = PSG_Get_Reg(i) & 0x3FF;
	}
	
	return MDP_ERR_OK;
}


int MDP_FNCALL mdp_host_reg_get_all(int icID, void *reg_struct)
{
	if (!Game)
		return -MDP_ERR_ROM_NOT_LOADED;
	if (!reg_struct)
		return -MDP_ERR_INVALID_PARAMETERS;
	
	switch (icID)
	{
		case MDP_REG_IC_M68K:
			return mdp_host_reg_get_all_68k(&main68k_context, reg_struct);
		case MDP_REG_IC_VDP:
			return mdp_host_reg_get_all_vdp(reg_struct);
		case MDP_REG_IC_YM2612:
			return mdp_host_reg_get_all_ym2612(reg_struct);
		case MDP_REG_IC_PSG:
			return mdp_host_reg_get_all_psg(reg_struct);
		case MDP_REG_IC_Z80:
		default:
			return -MDP_ERR_REG_INVALID_ICID;
	}
}
