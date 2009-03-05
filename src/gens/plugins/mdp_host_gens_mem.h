/***************************************************************************
 * Gens: MDP: Mega Drive Plugin - Host Services. (Memory Functions)        *
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

#ifndef GENS_MDP_HOST_GENS_MEM_H
#define GENS_MDP_HOST_GENS_MEM_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* MDP Host Services. */
#include "mdp/mdp.h"
#include "mdp/mdp_fncall.h"
#include "mdp/mdp_host.h"

/* Memory functions. */

uint8_t  MDP_FNCALL mdp_host_mem_read_8 (int memID, uint32_t address);
uint16_t MDP_FNCALL mdp_host_mem_read_16(int memID, uint32_t address);
uint32_t MDP_FNCALL mdp_host_mem_read_32(int memID, uint32_t address);

int MDP_FNCALL mdp_host_mem_write_8 (int memID, uint32_t address, uint8_t  data);
int MDP_FNCALL mdp_host_mem_write_16(int memID, uint32_t address, uint16_t data);
int MDP_FNCALL mdp_host_mem_write_32(int memID, uint32_t address, uint32_t data);

int MDP_FNCALL mdp_host_mem_read_block_8 (int memID, uint32_t address, uint8_t  *data, uint32_t length);
int MDP_FNCALL mdp_host_mem_read_block_16(int memID, uint32_t address, uint16_t *data, uint32_t length);
int MDP_FNCALL mdp_host_mem_read_block_32(int memID, uint32_t address, uint32_t *data, uint32_t length);

int MDP_FNCALL mdp_host_mem_write_block_8 (int memID, uint32_t address, uint8_t  *data, uint32_t length);
int MDP_FNCALL mdp_host_mem_write_block_16(int memID, uint32_t address, uint16_t *data, uint32_t length);
int MDP_FNCALL mdp_host_mem_write_block_32(int memID, uint32_t address, uint32_t *data, uint32_t length);

#ifdef __cplusplus
}
#endif

#endif /* GENS_MDP_HOST_GENS_MEM_H */
