/***************************************************************************
 * MDP: Mega Drive Plugins - Render Plugin Interface Definitions.          *
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

#ifndef __MDP_RENDER_H
#define __MDP_RENDER_H

#include <stdint.h>

#include "mdp_fncall.h"
#include "mdp_version.h"

#ifdef __cplusplus
extern "C" {
#endif

/* MDP Render Plugin interface version. */
#define MDP_RENDER_INTERFACE_VERSION MDP_VERSION(0, 1, 0)

/* Render information struct. */
typedef struct _mdp_render_info_t
{
	void *destScreen;	/* Destination screen buffer. */
	void *mdScreen;		/* Source screen buffer. */
	
	int destPitch;		/* Destination screen buffer pitch. */
	int srcPitch;		/* Source screen buffer pitch. */
	
	int width;		/* Image width. */
	int height;		/* Image height. */
	
	uint32_t bpp;		/* Current color depth. */
	
	uint32_t cpuFlags;	/* CPU flags. */
	uint32_t renderFlags;	/* Rendering flags. (Currently unused.) */
} mdp_render_info_t;

/* Render plugin flags. */

/**
 * SRC16DST32: Plugin only supports 16-bit color.
 * If the emulator is using 32-bit color, the emulator will have to
 * convert it from 16-bit to 32-bit manually.
 */
#define MDP_RENDER_FLAG_SRC16DST32	(1 << 0)

// Render plugin definition.
typedef int (MDP_FNCALL *mdp_render_fn)(mdp_render_info_t *renderInfo);
typedef struct _mdp_render_t
{
	const uint32_t interfaceVersion;	/* Render interface version. */
	mdp_render_fn blit;			/* Blit function. */
	const int scale;			/* Scaling ratio. (1 == 320x240; 2 = 640x480; etc) */
	const uint32_t flags;			/* Render flags. */
	const char* tag;			/* Render tag. */
} mdp_render_t;


#ifdef __cplusplus
}
#endif

#endif /* __MDP_RENDER_H */
