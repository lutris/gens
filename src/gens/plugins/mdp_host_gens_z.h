/***************************************************************************
 * Gens: MDP: Mega Drive Plugin - Host Services. (File Compression)        *
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

#ifndef GENS_MDP_HOST_GENS_Z_H
#define GENS_MDP_HOST_GENS_Z_H

#include <stdint.h>

/* MDP Host Services. */
#include "mdp/mdp.h"
#include "mdp/mdp_fncall.h"
#include "mdp/mdp_host.h"

#ifdef __cplusplus
extern "C" {
#endif

int MDP_FNCALL mdp_host_crc32(const uint8_t* buf, int length, uint32_t *crc32_out);

int MDP_FNCALL mdp_host_z_open(const char* filename, mdp_z_t **z_out);
int MDP_FNCALL mdp_host_z_get_file(mdp_z_t *z_file, mdp_z_entry_t *z_entry, void *buf, size_t size);
int MDP_FNCALL mdp_host_z_close(mdp_z_t *z_file);

#ifdef __cplusplus
}
#endif

#endif /* GENS_MDP_HOST_GENS_MEM_H */
