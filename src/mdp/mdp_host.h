/***************************************************************************
 * MDP: Mega Drive Plugin - Host Services.                                 *
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

#ifndef MDP_HOST_H
#define MDP_HOST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "mdp_fncall.h"
#include "mdp_version.h"
#include "mdp_render.h"

// MDP Host Services interface version.
#define MDP_HOST_INTERFACE_VERSION MDP_VERSION(0, 2, 0)

// MDP_PTR: Pointer IDs.
typedef enum
{
	MDP_PTR_NULL		= 0,
	MDP_PTR_LUT16to32	= 1,
	MDP_PTR_RGB16toYUV	= 2,
} MDP_PTR;

// MDP Host Services struct.
typedef struct
{
	const uint32_t interfaceVersion;
	
	// refPtr(): Get a reference for a pointer.
	// unrefPtr(): Unreference a pointer.
	void* (MDP_FNCALL *refPtr)(uint32_t ptrID);
	void  (MDP_FNCALL *unrefPtr)(uint32_t ptrID);
	
	// registerRenderer(): Register a renderer.
	// unregisterRenderer(): Unregister a renderer.
	int (MDP_FNCALL *registerRenderer)(struct MDP_t *plugin, MDP_Render_t *renderer);
	int (MDP_FNCALL *unregisterRenderer)(struct MDP_t *plugin, MDP_Render_t *renderer);
} MDP_Host_t;

#ifdef __cplusplus
}
#endif

#endif /* MDP_HOST_H */
