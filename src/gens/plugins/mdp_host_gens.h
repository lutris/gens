/***************************************************************************
 * Gens: MDP: Mega Drive Plugin - Host Services.                           *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
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

#ifndef GENS_MDP_HOST_GENS_H
#define GENS_MDP_HOST_GENS_H

#ifdef __cplusplus
extern "C" {
#endif

// MDP Host Services.
#include "mdp/mdp.h"	// TODO: Fix the typedef struct MDP_t hack for mdp_host.h
#include "mdp/mdp_fncall.h"
#include "mdp/mdp_host.h"

// MDP Host Services struct.
extern MDP_Host_t Gens_MDP_Host;

// Host service functions.
void* MDP_FNCALL mdp_host_ref_ptr(uint32_t ptrID);
void  MDP_FNCALL mdp_host_unref_ptr(uint32_t ptrID);

int MDP_FNCALL registerRenderer(struct MDP_t *plugin, MDP_Render_t *renderer);
int MDP_FNCALL unregisterRenderer(struct MDP_t *plugin, MDP_Render_t *renderer);

#ifdef __cplusplus
}
#endif

#endif /* GENS_MDP_HOST_GENS_H */
