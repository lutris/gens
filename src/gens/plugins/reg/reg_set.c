/***************************************************************************
 * Gens: MDP: Mega Drive Plugin - Host Services. (reg_set() functions)     *
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
#include "libgsft/gsft_unused.h"

#include "reg_set.h"
#include "mdp/mdp_error.h"
#include "mdp/mdp_reg.h"

#include "util/file/rom.hpp"

#include "gens_core/cpu/68k/star_68k.h"
#include "gens_core/vdp/vdp_io.h"
#include "gens_core/sound/ym2612.hpp"
#include "gens_core/sound/psg.h"
#include "mdZ80/mdZ80.h"

extern int ice;


/** reg_set() **/


/**
 * mdp_host_reg_get_68k(): Set a 68000 register.
 * @param context 68000 context.
 * @param regID Register ID.
 * @param new_value New value for the register.
 * @return MDP error code.
 */
static int mdp_host_reg_set_68k(struct S68000CONTEXT *context, int regID, uint32_t new_value)
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
			context->dreg[regID - MDP_REG_68K_D0] = new_value;
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
			context->areg[regID - MDP_REG_68K_A0] = new_value;
			break;
		
		case MDP_REG_68K_ASP:
			context->asp = new_value;
			break;
		
		case MDP_REG_68K_PC:
			context->pc = new_value;
			break;
		
		case MDP_REG_68K_SR:
			context->sr = (uint16_t)new_value;
			break;
		
		default:
			return -MDP_ERR_REG_INVALID_REGID;
	}
	
	return MDP_ERR_OK;
}


/**
 * mdp_host_reg_set_vdp(): Set a VDP register.
 * @param regID Register ID.
 * @param new_value New value for the register.
 * @return MDP error code.
 */
static int mdp_host_reg_set_vdp(int regID, uint32_t new_value)
{
	if (regID < 0 || regID >= 24)
		return -MDP_ERR_REG_INVALID_REGID;
	
	Set_VDP_Reg(regID, (new_value & 0xFF));
	return MDP_ERR_OK;
}


/**
 * mdp_host_reg_set_ym2612(): Set a YM2612 register.
 * @param regID Register ID.
 * @param new_value New value for the register.
 * @return MDP error code.
 */
static int mdp_host_reg_set_ym2612(int regID, uint32_t new_value)
{
	if (regID < 0 || regID >= 0x200)
		return -MDP_ERR_REG_INVALID_REGID;
	
	const int base_addr = MDP_REG_YM2612_GET_BANK(regID) * 2;
	new_value &= 0xFF;
	YM2612_Write(base_addr, MDP_REG_YM2612_GET_REG(regID));
	YM2612_Write(base_addr + 1, new_value);
	return MDP_ERR_OK;
}


/**
 * mdp_host_reg_set_psg(): Set a PSG register.
 * @param regID Register ID.
 * @param new_value New value for the register.
 * @return MDP error code.
 */
static int mdp_host_reg_set_psg(int regID, uint32_t new_value)
{
	if (regID < 0 || regID >= 8)
		return -MDP_ERR_REG_INVALID_REGID;
	
	PSG_Write(0x80 | (regID << 4) | (new_value & 0x0F));
	PSG_Write((new_value >> 4) & 0x3F);
	return MDP_ERR_OK;
}


/**
 * mdp_host_reg_set_z80(): Set a Z80 register.
 * @param regID Register ID.
 * @param new_value New value for the register.
 * @return MDP error code.
 */
static int mdp_host_reg_set_z80(int regID, uint32_t new_value)
{
	unsigned int tmp;
	
	switch (regID)
	{
		/* Main registers: BYTE */
		case MDP_REG_Z80_A:
			tmp = mdZ80_get_AF(&M_Z80);
			tmp &= 0xFF; // Save F.
			tmp |= ((new_value & 0xFF) << 8);
			mdZ80_set_AF(&M_Z80, tmp);
			break;
		case MDP_REG_Z80_F:
			tmp = mdZ80_get_AF(&M_Z80);
			tmp &= 0xFF00; // Save A.
			tmp |= (new_value & 0xFF);
			mdZ80_set_AF(&M_Z80, tmp);
			break;
		case MDP_REG_Z80_B:
			M_Z80.BC.b.B = (uint8_t)new_value;
			break;
		case MDP_REG_Z80_C:
			M_Z80.BC.b.C = (uint8_t)new_value;
			break;
		case MDP_REG_Z80_D:
			M_Z80.DE.b.D = (uint8_t)new_value;
			break;
		case MDP_REG_Z80_E:
			M_Z80.DE.b.E = (uint8_t)new_value;
			break;
		case MDP_REG_Z80_H:
			M_Z80.HL.b.H = (uint8_t)new_value;
			break;
		case MDP_REG_Z80_L:
			M_Z80.HL.b.L = (uint8_t)new_value;
			break;
		case MDP_REG_Z80_IXH:
			M_Z80.IX.b.IXH = (uint8_t)new_value;
			break;
		case MDP_REG_Z80_IXL:
			M_Z80.IX.b.IXL = (uint8_t)new_value;
			break;
		case MDP_REG_Z80_IYH:
			M_Z80.IY.b.IYH = (uint8_t)new_value;
			break;
		case MDP_REG_Z80_IYL:
			M_Z80.IY.b.IYL = (uint8_t)new_value;
			break;
		
		/* Main registers: WORD */
		case MDP_REG_Z80_AF:
			mdZ80_set_AF(&M_Z80, new_value);
			break;
		case MDP_REG_Z80_BC:
			M_Z80.BC.w.BC = (uint16_t)new_value;
			break;
		case MDP_REG_Z80_DE:
			M_Z80.DE.w.DE = (uint16_t)new_value;
			break;
		case MDP_REG_Z80_HL:
			M_Z80.HL.w.HL = (uint16_t)new_value;
			break;
		case MDP_REG_Z80_IX:
			M_Z80.IX.w.IX = (uint16_t)new_value;
			break;
		case MDP_REG_Z80_IY:
			M_Z80.IY.w.IY = (uint16_t)new_value;
			break;
		
		/* Shadow registers: BYTE */
		case MDP_REG_Z80_A2:
			tmp = mdZ80_get_AF2(&M_Z80);
			tmp &= 0xFF; // Save F.
			tmp |= ((new_value & 0xFF) << 8);
			mdZ80_set_AF2(&M_Z80, tmp);
			break;
		case MDP_REG_Z80_F2:
			tmp = mdZ80_get_AF2(&M_Z80);
			tmp &= 0xFF00; // Save A.
			tmp |= (new_value & 0xFF);
			mdZ80_set_AF2(&M_Z80, tmp);
			break;
		case MDP_REG_Z80_B2:
			M_Z80.BC2.b.B2 = (uint8_t)new_value;
			break;
		case MDP_REG_Z80_C2:
			M_Z80.BC2.b.C2 = (uint8_t)new_value;
			break;
		case MDP_REG_Z80_D2:
			M_Z80.DE2.b.D2 = (uint8_t)new_value;
			break;
		case MDP_REG_Z80_E2:
			M_Z80.DE2.b.E2 = (uint8_t)new_value;
			break;
		case MDP_REG_Z80_H2:
			M_Z80.HL2.b.H2 = (uint8_t)new_value;
			break;
		case MDP_REG_Z80_L2:
			M_Z80.HL2.b.L2 = (uint8_t)new_value;
			break;
		
		/* Shadow registers: WORD */
		case MDP_REG_Z80_AF2:
			mdZ80_set_AF2(&M_Z80, new_value);
			break;
		case MDP_REG_Z80_BC2:
			M_Z80.BC2.w.BC2 = (uint16_t)new_value;
			break;
		case MDP_REG_Z80_DE2:
			M_Z80.DE2.w.DE2 = (uint16_t)new_value;
			break;
		case MDP_REG_Z80_HL2:
			M_Z80.HL2.w.HL2 = (uint16_t)new_value;
			break;
		
		/* Other registers. */
		case MDP_REG_Z80_PC:
			mdZ80_set_PC(&M_Z80, new_value);
			break;
		case MDP_REG_Z80_SP:
			M_Z80.SP.w.SP = new_value;
			break;
		case MDP_REG_Z80_I:
			M_Z80.I = new_value & 0xFF;
			break;
		case MDP_REG_Z80_R:
			M_Z80.R.b.R1 = new_value;
			break;
		case MDP_REG_Z80_IM:
			M_Z80.IM = new_value & 3;
			break;
		case MDP_REG_Z80_IFF:
			M_Z80.IFF.b.IFF1 = (new_value & 1);
			M_Z80.IFF.b.IFF2 = ((new_value & 2) >> 1);
			break;
		
		default:
			return -MDP_ERR_REG_INVALID_REGID;
	}
	
	return MDP_ERR_OK;
}


/**
 * mdp_host_reg_set(): Set a register.
 * @param plugin Plugin requesting register set.
 * @param icID IC ID.
 * @param regID Register ID.
 * @param new_value New value for the register.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_host_reg_set(mdp_t *plugin, int icID, int regID, uint32_t new_value)
{
	GSFT_UNUSED_PARAMETER(plugin);
	
	if (!Game)
		return -MDP_ERR_ROM_NOT_LOADED;
	
	switch (icID)
	{
		case MDP_REG_IC_M68K:
			return mdp_host_reg_set_68k(&main68k_context, regID, new_value);
		case MDP_REG_IC_VDP:
			return mdp_host_reg_set_vdp(regID, new_value);
		case MDP_REG_IC_YM2612:
			return mdp_host_reg_set_ym2612(regID, new_value);
		case MDP_REG_IC_PSG:
			return mdp_host_reg_set_psg(regID, new_value);
		case MDP_REG_IC_Z80:
			return mdp_host_reg_set_z80(regID, new_value);
		default:
			return -MDP_ERR_REG_INVALID_ICID;
	}
}
