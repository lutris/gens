/***************************************************************************
 * MDP: Mega Drive Plugins - Event Handler Declarations.                   *
 *                                                                         *
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

#ifndef __MDP_EVENT_H
#define __MDP_EVENT_H

#include "mdp_fncall.h"
#include "mdp_stdint.h"
#include "mdp_constants.h"

#ifdef __cplusplus
extern "C" {
#endif


/* MDP Event IDs */
typedef enum _MDP_EVENT_ID
{
	/*! BEGIN: MDP v1.0 Event IDs. !*/
	MDP_EVENT_UNKNOWN	= 0,
	MDP_EVENT_LOAD_CONFIG	= 1,	/* event_info == NULL */
	MDP_EVENT_SAVE_CONFIG	= 2,	/* event_info == NULL */
	MDP_EVENT_OPEN_ROM	= 3,	/* event_info == mdp_event_open_rom_t */
	MDP_EVENT_CLOSE_ROM	= 4,	/* event_info == NULL */
	MDP_EVENT_PRE_FRAME	= 5,	/* event_info == NULL */
	MDP_EVENT_POST_FRAME	= 6,	/* event_info == mdp_event_post_frame_t */
	/*! END: MDP v1.0 Event IDs. !*/
	
	MDP_EVENT_MAX			/* Maximum number of events. */
} MDP_EVENT_ID;


/**
 * mdp_event_callback_fn(): Event callback function.
 * @param event_id Event ID.
 * @param event_info Pointer to struct containing event information.
 * This should be casted to an appropriate struct, based on event_id.
 * This may be NULL, depending on if the event uses a struct or not.
 * @return MDP error code.
 */
typedef int (MDP_FNCALL *mdp_event_handler_fn)(int event_id, void *event_info);


/*! BEGIN: MDP v1.0 Event Information Structs. !*/


/**
 * mdp_event_open_rom_t: Event information for when a ROM is opened.
 */
#pragma pack(1)
typedef struct PACKED _mdp_event_open_rom_t
{
	const char *rom_name;	/* ROM name. (For archives, the name of the archive.) */
	
	const char *rom_z_name;	/* If the ROM is in a multi-file compressed archive, this *
				 * is the name of the ROM file from inside the archive.   *
				 * If not, this is NULL.                                  */
	
	int system_id;		/* System ID. */
} mdp_event_open_rom_t;
#pragma pack()


/**
 * mdp_event_post_frame_t: Event raised when a frame has been drawn to the internal framebuffer.
 */
#pragma pack(1)
typedef struct PACKED _mdp_event_post_frame_t
{
	void    *md_screen;	/* MD frame buffer. */
	
	uint32_t width;		/* Width of the image. */
	uint32_t height;	/* Height of the image. */
	
	uint32_t pitch;		/* Pitch. (bytes per scanline) */
	uint32_t bpp;		/* Bits per pixel. */
} mdp_event_post_frame_t;
#pragma pack()


/*! END: MDP v1.0 Event Information Structs. !*/


#ifdef __cplusplus
}
#endif

#endif /* __MDP_EVENT_H */
