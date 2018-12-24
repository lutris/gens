/***************************************************************************
 * Gens: MDP: Mega Drive Plugin - Host Services. (reg_get() functions)     *
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

#include "reg_get.h"
#include "mdp/mdp_error.h"
#include "mdp/mdp_reg.h"

#include "util/file/rom.hpp"

#include "gens_core/cpu/68k/star_68k.h"
#include "gens_core/vdp/vdp_io.h"
#include "gens_core/sound/ym2612.hpp"
#include "gens_core/sound/psg.h"
#include "mdZ80/mdZ80.h"


/**
 * mdp_host_reg_get_68k(): Get a 68000 register.
 * @param context 68000 context.
 * @param regID Register ID.
 * @param ret_value Pointer to variable to store the register in.
 * @return MDP error code.
 */
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


/**
 * mdp_host_reg_get_vdp(): Get a VDP register.
 * @param regID Register ID.
 * @param ret_value Pointer to variable to store the register in.
 * @return MDP error code.
 */
static int mdp_host_reg_get_vdp(int regID, uint32_t *ret_value)
{
	if (regID < 0 || regID >= 24)
		return -MDP_ERR_REG_INVALID_REGID;
	
	// VDP_Reg is an array of 24 unsigned ints.
	unsigned int *vdp_reg_int = (unsigned int*)(&VDP_Reg);
	*ret_value = vdp_reg_int[regID] & 0xFF;
	return MDP_ERR_OK;
}


/**
 * mdp_host_reg_get_ym2612(): Get a YM2612 register.
 * @param regID Register ID.
 * @param ret_value Pointer to variable to store the register in.
 * @return MDP error code.
 */
static int mdp_host_reg_get_ym2612(int regID, uint32_t *ret_value)
{
	if (regID < 0 || regID >= 0x200)
		return -MDP_ERR_REG_INVALID_REGID;
	
	*ret_value = YM2612_Get_Reg(regID) & 0xFF;
	return MDP_ERR_OK;
}


/**
 * mdp_host_reg_get_psg(): Get a PSG register.
 * @param regID Register ID.
 * @param ret_value Pointer to variable to store the register in.
 * @return MDP error code.
 */
static int mdp_host_reg_get_psg(int regID, uint32_t *ret_value)
{
	if (regID < 0 || regID >= 8)
		return -MDP_ERR_REG_INVALID_REGID;
	
	*ret_value = PSG_Get_Reg(regID) & 0x3FF;
	return MDP_ERR_OK;
}


/**
 * mdp_host_reg_get_z80(): Get a Z80 register.
 * @param regID Register ID.
 * @param ret_value Pointer to variable to store the register in.
 * @return MDP error code.
 */
static int mdp_host_reg_get_z80(int regID, uint32_t *ret_value)
{
	unsigned int tmp;
	
	switch (regID)
	{
		/* Main registers: BYTE */
		case MDP_REG_Z80_A:
			tmp = mdZ80_get_AF(&M_Z80);
			*ret_value = ((tmp >> 8) & 0xFF);
			break;
		case MDP_REG_Z80_F:
			tmp = mdZ80_get_AF(&M_Z80);
			*ret_value = (tmp & 0xFF);
			break;
		case MDP_REG_Z80_B:
			*ret_value = M_Z80.BC.b.B;
			break;
		case MDP_REG_Z80_C:
			*ret_value = M_Z80.BC.b.C;
			break;
		case MDP_REG_Z80_D:
			*ret_value = M_Z80.DE.b.D;
			break;
		case MDP_REG_Z80_E:
			*ret_value = M_Z80.DE.b.E;
			break;
		case MDP_REG_Z80_H:
			*ret_value = M_Z80.HL.b.H;
			break;
		case MDP_REG_Z80_L:
			*ret_value = M_Z80.HL.b.L;
			break;
		case MDP_REG_Z80_IXH:
			*ret_value = M_Z80.IX.b.IXH;
			break;
		case MDP_REG_Z80_IXL:
			*ret_value = M_Z80.IX.b.IXL;
			break;
		case MDP_REG_Z80_IYH:
			*ret_value = M_Z80.IY.b.IYH;
			break;
		case MDP_REG_Z80_IYL:
			*ret_value = M_Z80.IY.b.IYL;
			break;
		
		/* Main registers: WORD */
		case MDP_REG_Z80_AF:
			*ret_value = mdZ80_get_AF(&M_Z80);
			break;
		case MDP_REG_Z80_BC:
			*ret_value = M_Z80.BC.w.BC;
			break;
		case MDP_REG_Z80_DE:
			*ret_value = M_Z80.DE.w.DE;
			break;
		case MDP_REG_Z80_HL:
			*ret_value = M_Z80.HL.w.HL;
			break;
		case MDP_REG_Z80_IX:
			*ret_value = M_Z80.IX.w.IX;
			break;
		case MDP_REG_Z80_IY:
			*ret_value = M_Z80.IY.w.IY;
			break;
		
		/* Shadow registers: BYTE */
		case MDP_REG_Z80_A2:
			tmp = mdZ80_get_AF2(&M_Z80);
			*ret_value = ((tmp >> 8) & 0xFF);
			break;
		case MDP_REG_Z80_F2:
			tmp = mdZ80_get_AF2(&M_Z80);
			*ret_value = (tmp & 0xFF);
			break;
		case MDP_REG_Z80_B2:
			*ret_value = M_Z80.BC2.b.B2;
			break;
		case MDP_REG_Z80_C2:
			*ret_value = M_Z80.BC2.b.C2;
			break;
		case MDP_REG_Z80_D2:
			*ret_value = M_Z80.DE2.b.D2;
			break;
		case MDP_REG_Z80_E2:
			*ret_value = M_Z80.DE2.b.E2;
			break;
		case MDP_REG_Z80_H2:
			*ret_value = M_Z80.HL2.b.H2;
			break;
		case MDP_REG_Z80_L2:
			*ret_value = M_Z80.HL2.b.L2;
			break;
		
		/* Shadow registers: WORD */
		case MDP_REG_Z80_AF2:
			*ret_value = mdZ80_get_AF2(&M_Z80);
			break;
		case MDP_REG_Z80_BC2:
			*ret_value = M_Z80.BC2.w.BC2;
			break;
		case MDP_REG_Z80_DE2:
			*ret_value = M_Z80.DE2.w.DE2;
			break;
		case MDP_REG_Z80_HL2:
			*ret_value = M_Z80.HL2.w.HL2;
			break;
		
		/* Other registers. */
		case MDP_REG_Z80_PC:
			*ret_value = mdZ80_get_PC(&M_Z80);
			break;
		case MDP_REG_Z80_SP:
			*ret_value = M_Z80.SP.w.SP;
			break;
		case MDP_REG_Z80_I:
			*ret_value = M_Z80.I & 0xFF;
			break;
		case MDP_REG_Z80_R:
			*ret_value = M_Z80.R.b.R1;
			break;
		case MDP_REG_Z80_IM:
			*ret_value = M_Z80.IM & 3;
			break;
		case MDP_REG_Z80_IFF:
			*ret_value = (M_Z80.IFF.b.IFF2 ? 2 : 0) | (M_Z80.IFF.b.IFF1 ? 1 : 0);
			break;
		
		default:
			return -MDP_ERR_REG_INVALID_REGID;
	}
	
	return MDP_ERR_OK;
}


/**
 * mdp_host_reg_get(): Get a register.
 * @param icID IC ID.
 * @param regID Register ID.
 * @param ret_value Pointer to variable to store the register in.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_host_reg_get(int icID, int regID, uint32_t *ret_value)
{
	if (!Game)
		return -MDP_ERR_ROM_NOT_LOADED;
	if (!ret_value)
		return -MDP_ERR_INVALID_PARAMETERS;
	
	switch (icID)
	{
		case MDP_REG_IC_M68K:
			return mdp_host_reg_get_68k(&main68k_context, regID, ret_value);
		case MDP_REG_IC_VDP:
			return mdp_host_reg_get_vdp(regID, ret_value);
		case MDP_REG_IC_YM2612:
			return mdp_host_reg_get_ym2612(regID, ret_value);
		case MDP_REG_IC_PSG:
			return mdp_host_reg_get_psg(regID, ret_value);
		case MDP_REG_IC_Z80:
			return mdp_host_reg_get_z80(regID, ret_value);
		default:
			return -MDP_ERR_REG_INVALID_ICID;
	}
}
