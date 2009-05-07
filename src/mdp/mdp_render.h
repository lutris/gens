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

#include "mdp_fncall.h"
#include "mdp_version.h"
#include "mdp_stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

/* MDP Render Plugin interface version. */
#define MDP_RENDER_INTERFACE_VERSION MDP_VERSION(0, 1, 0)

/* Video mode flags. */

/* Bit 0: 555/565 mode (16-bit color) */
#define MDP_RENDER_VMODE_RGB_MODE		((uint32_t)(1 << 0))
#define MDP_RENDER_VMODE_RGB_555		((uint32_t)(0))
#define MDP_RENDER_VMODE_RGB_565		((uint32_t)(1 << 0))
/* If 32-bit color, this bit is used to indicate if the source is 565 or 888. */
#define MDP_RENDER_VMODE_RGB_SRC888DST888	((uint32_t)(0))
#define MDP_RENDER_VMODE_RGB_SRC565DST888	((uint32_t)(1 << 0))

/* Bit 1: 16/32-bit color. */
#define MDP_RENDER_VMODE_BPP		((uint32_t)(1 << 1))
#define MDP_RENDER_VMODE_BPP_16		((uint32_t)(0))
#define MDP_RENDER_VMODE_BPP_32		((uint32_t)(1 << 1))

/* Render information struct. */
#pragma pack(1)
typedef struct PACKED _mdp_render_info_t
{
	void *destScreen;	/* Destination screen buffer. */
	void *mdScreen;		/* Source screen buffer. */
	
	int destPitch;		/* Destination screen buffer pitch. */
	int srcPitch;		/* Source screen buffer pitch. */
	
	int width;		/* Image width. */
	int height;		/* Image height. */
	
	uint32_t cpuFlags;	/* CPU flags. */
	uint32_t vmodeFlags;	/* Video mode flags. */
	
	void *data;		/* Extra data set by the plugin. */
} mdp_render_info_t;
#pragma pack()

/* Render plugin flags. */
#define MDP_RENDER_FLAG_RGB555		((uint32_t)(1 << 0))
#define MDP_RENDER_FLAG_RGB565		((uint32_t)(1 << 1))
#define MDP_RENDER_FLAG_RGB888		((uint32_t)(1 << 2))
#define MDP_RENDER_FLAG_SRC565DST888	((uint32_t)(1 << 3))

// Render plugin definition.
typedef int (MDP_FNCALL *mdp_render_fn)(mdp_render_info_t *renderInfo);

#pragma pack(1)
typedef struct PACKED _mdp_render_t
{
	const uint32_t interfaceVersion;	/* Render interface version. */
	const int scale;			/* Scaling ratio. (1 == 320x240; 2 = 640x480; etc) */
	
	mdp_render_fn blit;			/* Blit function. */
	const char* tag;			/* Render tag. */
	
	const uint32_t flags;			/* Render flags. */
	
	void *data;				/* Extra data. */
} mdp_render_t;
#pragma pack()

#ifdef __cplusplus
}
#endif

#endif /* __MDP_RENDER_H */
