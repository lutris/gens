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
#include "libgsft/gsft_unused.h"

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
 * mdp_host_reg_set_all_68k(): Set all 68000 registers.
 * @param context 68000 context.
 * @param reg_struct Pointer to mdp_reg_68k_t struct to get the registers from.
 * @return MDP error code.
 */
static int mdp_host_reg_set_all_68k(struct S68000CONTEXT *context, void *reg_struct)
{
	mdp_reg_68k_t *reg_68k = (mdp_reg_68k_t*)reg_struct;
	int i;
	
	for (i = 0; i < 8; i++)
	{
		context->dreg[i] = reg_68k->dreg[i];
		context->areg[i] = reg_68k->areg[i];
	}
	
	context->asp = reg_68k->asp;
	context->pc = reg_68k->pc;
	context->sr = reg_68k->sr;
	
	return MDP_ERR_OK;
}


/**
 * mdp_host_reg_set_all_vdp(): Set all VDP registers.
 * @param reg_struct Pointer to mdp_reg_vdp_t struct to get the registers from.
 * @return MDP error code.
 */
static int mdp_host_reg_set_all_vdp(void *reg_struct)
{
	mdp_reg_vdp_t *reg_vdp = (mdp_reg_vdp_t*)reg_struct;
	
	int i;
	for (i = 0; i < 24; i++)
	{
		Set_VDP_Reg(i, reg_vdp->data[i]);
	}
	
	return MDP_ERR_OK;
}


/**
 * mdp_host_reg_set_all_ym2612(): Set all YM2612 registers.
 * @param reg_struct Pointer to mdp_reg_ym2612_t struct to get the registers from.
 * @return MDP error code.
 */
static int mdp_host_reg_set_all_ym2612(void *reg_struct)
{
	mdp_reg_ym2612_t *reg_ym2612 = (mdp_reg_ym2612_t*)reg_struct;
	
	int i;
	for (i = 0; i < 0x100; i++)
	{
		YM2612_Write(0, i);
		YM2612_Write(1, reg_ym2612->regs[0][i]);
		
		YM2612_Write(2, i);
		YM2612_Write(3, reg_ym2612->regs[1][i]);
	}
	
	return MDP_ERR_OK;
}


/**
 * mdp_host_reg_set_all_psg(): Set all PSG registers.
 * @param reg_struct Pointer to mdp_reg_psg_t struct to get the registers from.
 * @return MDP error code.
 */
static int mdp_host_reg_set_all_psg(void *reg_struct)
{
	mdp_reg_psg_t *reg_psg = (mdp_reg_psg_t*)reg_struct;
	
	int i;
	for (i = 0; i < 8; i++)
	{
		PSG_Write(0x80 | (i << 4) | (reg_psg->regs[i] & 0x0F));
		PSG_Write((reg_psg->regs[i] >> 4) & 0x3F);
	}
	
	return MDP_ERR_OK;
}


/**
 * mdp_host_reg_set_all_z80(): Set all Z80 registers.
 * @param reg_struct Pointer to mdp_reg_psg_t struct to get the registers from.
 * @return MDP error code.
 */
static int mdp_host_reg_set_all_z80(void *reg_struct)
{
	mdp_reg_z80_t *reg_z80 = (mdp_reg_z80_t*)reg_struct;
	
	/* Main registers. */
	mdZ80_set_AF(&M_Z80, reg_z80->AF);
	M_Z80.BC.w.BC = reg_z80->BC;
	M_Z80.DE.w.DE = reg_z80->DE;
	M_Z80.HL.w.HL = reg_z80->HL;
	M_Z80.IX.w.IX = reg_z80->IX;
	M_Z80.IY.w.IY = reg_z80->IY;
	
	/* Shadow registers. */
	mdZ80_set_AF2(&M_Z80, reg_z80->AF2);
	M_Z80.BC2.w.BC2 = reg_z80->BC2;
	M_Z80.DE2.w.DE2 = reg_z80->DE2;
	M_Z80.HL2.w.HL2 = reg_z80->HL2;
	
	/* Other registers. */
	mdZ80_set_PC(&M_Z80, reg_z80->PC);
	M_Z80.I = reg_z80->I;
	M_Z80.R.b.R1 = reg_z80->R;
	M_Z80.IM = reg_z80->IM & 3;
	M_Z80.IFF.b.IFF1 = (reg_z80->IFF & 1);
	M_Z80.IFF.b.IFF2 = ((reg_z80->IFF & 2) >> 1);
	
	return MDP_ERR_OK;
}


/**
 * mdp_host_reg_set_all(): Set all registers for the specified IC.
 * @param icID IC ID.
 * @param reg_struct Pointer to register structure for the specific IC.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_host_reg_set_all(mdp_t *plugin, int icID, void *reg_struct)
{
	GSFT_UNUSED_PARAMETER(plugin);
	
	if (!Game)
		return -MDP_ERR_ROM_NOT_LOADED;
	if (!reg_struct)
		return -MDP_ERR_INVALID_PARAMETERS;
	
	switch (icID)
	{
		case MDP_REG_IC_M68K:
			return mdp_host_reg_set_all_68k(&main68k_context, reg_struct);
		case MDP_REG_IC_VDP:
			return mdp_host_reg_set_all_vdp(reg_struct);
		case MDP_REG_IC_YM2612:
			return mdp_host_reg_set_all_ym2612(reg_struct);
		case MDP_REG_IC_PSG:
			return mdp_host_reg_set_all_psg(reg_struct);
		case MDP_REG_IC_Z80:
			return mdp_host_reg_set_all_z80(reg_struct);
		default:
			return -MDP_ERR_REG_INVALID_ICID;
	}
}
