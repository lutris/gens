/***************************************************************************
 * MDP: VDP Debugger. (Mode 5 Register Descriptions)                       *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2010 by David Korth                                  *
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

#ifndef _MDP_MISC_VDPDBG_VDP_REG_M5_H
#define _MDP_MISC_VDPDBG_VDP_REG_M5_H

#include "mdp/mdp.h"
#include "mdp/mdp_fncall.h"
#include "mdp/mdp_reg.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char *vdp_m5_reg_name[24+1];

DLL_LOCAL void MDP_FNCALL vdpdbg_get_m5_reg_desc(int reg_num, uint8_t reg_value, mdp_reg_vdp_t *reg_vdp, char *buf, size_t len);

DLL_LOCAL void MDP_FNCALL vdpdbg_get_m5_dma_len_desc(mdp_reg_vdp_t *reg_vdp, char *buf, size_t len);
DLL_LOCAL void MDP_FNCALL vdpdbg_get_m5_dma_src_desc(mdp_reg_vdp_t *reg_vdp, char *buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* _MDP_MISC_VDPDBG_VDP_REG_M5_H */
