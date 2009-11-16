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

/* Video mode flags. */

/*! BEGIN: MDP v1.0 video mode flags. !*/

#define MDP_RENDER_VMODE_CREATE(src, dst)	\
	(((src) & 0x03) | (((dst) & 0x03) << 2))

/* Color modes. */
#define MDP_RENDER_VMODE_RGB_555		((uint32_t)(0x00))
#define MDP_RENDER_VMODE_RGB_565		((uint32_t)(0x01))
#define MDP_RENDER_VMODE_RGB_888		((uint32_t)(0x02))

/* Bits 0-1: Source color mode. */
#define MDP_RENDER_VMODE_GET_SRC(x)	(x & 0x03)
/* Bits 2-3: Destination color mode. */
#define MDP_RENDER_VMODE_GET_DST(x)	((x >> 2) & 0x03)

/*! END: MDP v1.0 video mode flags. !*/

/** Render information struct. **/
#pragma pack(1)
typedef struct PACKED _mdp_render_info_t
{
	/*! BEGIN: MDP v1.0 render information struct. !*/
	
	void *destScreen;	/* Destination screen buffer. */
	void *mdScreen;		/* Source screen buffer. */
	
	int destPitch;		/* Destination screen buffer pitch. */
	int srcPitch;		/* Source screen buffer pitch. */
	
	int width;		/* Image width. */
	int height;		/* Image height. */
	
	uint32_t cpuFlags;	/* CPU flags. */
	uint32_t vmodeFlags;	/* Video mode flags. */
	
	void *data;		/* Extra data set by the plugin. */
	
	/*! END: MDP v1.0 render information struct. !*/
} mdp_render_info_t;
#pragma pack()

/** Render plugin flags. **/
/*! BEGIN: MDP v1.0 render plugin flags. !*/
#define MDP_RENDER_FLAG_RGB_555to555	((uint32_t)(1 << 0))
#define MDP_RENDER_FLAG_RGB_555to565	((uint32_t)(1 << 1))
#define MDP_RENDER_FLAG_RGB_555to888	((uint32_t)(1 << 2))
#define MDP_RENDER_FLAG_RGB_565to555	((uint32_t)(1 << 3))
#define MDP_RENDER_FLAG_RGB_565to565	((uint32_t)(1 << 4))
#define MDP_RENDER_FLAG_RGB_565to888	((uint32_t)(1 << 5))
#define MDP_RENDER_FLAG_RGB_888to555	((uint32_t)(1 << 6))
#define MDP_RENDER_FLAG_RGB_888to565	((uint32_t)(1 << 7))
#define MDP_RENDER_FLAG_RGB_888to888	((uint32_t)(1 << 8))
/*! END: MDP v1.0 render plugin flags. !*/

/* Render plugin function definition. */
typedef int (MDP_FNCALL *mdp_render_fn)(const mdp_render_info_t *renderInfo);

/** Render plugin definition struct. */
#pragma pack(1)
typedef struct PACKED _mdp_render_t
{
	/*! BEGIN: MDP v1.0 render plugin definition struct. !*/
	
	mdp_render_fn blit;	/* Blit function. */
	const char* tag;	/* Render tag. */
	
	int scale;		/* Scaling ratio. (1 == 320x240; 2 = 640x480; etc) */
	uint32_t flags;		/* Render flags. */
	
	void *data;		/* Extra data. */
	
	/*! END: MDP v1.0 render plugin definition struct. !*/
} mdp_render_t;
#pragma pack()

#ifdef __cplusplus
}
#endif

#endif /* __MDP_RENDER_H */
