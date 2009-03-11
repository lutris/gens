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

/* Video mode flags. */

/* Bit 0: 555/565 mode (16-bit color) */
#define MDP_RENDER_VMODE_RGB_MODE	(1 << 0)
#define MDP_RENDER_VMODE_RGB_555	0
#define MDP_RENDER_VMODE_RGB_565	(1 << 0)

/* Bit 1: 16/32-bit color. */
#define MDP_RENDER_VMODE_BPP		(1 << 1)
#define MDP_RENDER_VMODE_BPP_16		0
#define MDP_RENDER_VMODE_BPP_32		(1 << 1)

/* Render information struct. */
typedef struct _mdp_render_info_t
{
	void *destScreen;	/* Destination screen buffer. */
	void *mdScreen;		/* Source screen buffer. */
	
	int destPitch;		/* Destination screen buffer pitch. */
	int srcPitch;		/* Source screen buffer pitch. */
	
	int width;		/* Image width. */
	int height;		/* Image height. */
	
	uint32_t cpuFlags;	/* CPU flags. */
	uint32_t vmodeFlags;	/* Video mode flags. */
} mdp_render_info_t;

/* Render plugin flags. */
#define MDP_RENDER_FLAG_RGB555		(1 << 0)
#define MDP_RENDER_FLAG_RGB565		(1 << 1)
#define MDP_RENDER_FLAG_RGB888		(1 << 2)

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
