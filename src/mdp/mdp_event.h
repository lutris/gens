/***************************************************************************
 * Gens: MDP: Mega Drive Plugin - Event Handler Declarations.              *
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

#ifndef __MDP_EVENT_H
#define __MDP_EVENT_H

#include <stdint.h>
#include "mdp_fncall.h"
#include "mdp_constants.h"

#ifdef __cplusplus
extern "C" {
#endif


// MDP Event IDs
enum MDP_EVENT_ID
{
	MDP_EVENT_UNKNOWN	= 0,
	MDP_EVENT_OPEN_ROM	= 1,	// event_info == mdp_event_open_rom_t
	MDP_EVENT_CLOSE_ROM	= 2,	// event_info == NULL
	MDP_EVENT_PRE_FRAME	= 3,	// event_info == NULL
	MDP_EVENT_POST_FRAME	= 4,	// event_info == mdp_event_post_frame_t
	
	MDP_EVENT_MAX			// Maximum number of events.
};


/**
 * mdp_event_callback_fn(): Event callback function.
 * @param event_id Event ID.
 * @param event_info Pointer to struct containing event information.
 * This should be casted to an appropriate struct, based on event_id.
 * This may be NULL, depending on if the event uses a struct or not.
 * @return MDP error code.
 */
typedef int (MDP_FNCALL *mdp_event_handler_fn)(int event_id, void *event_info);


/**
 * mdp_event_open_rom_t: Event information for when a ROM is opened.
 */
typedef struct _mdp_event_open_rom_t
{
	const char *rom_name;	// ROM name.
	int system_id;		// System ID.
} mdp_event_open_rom_t;


/**
 * mdp_event_post_frame_t: Event raised when a frame has been drawn to the internal framebuffer.
 */
typedef struct _mdp_event_post_frame_t
{
	void    *md_screen;	// MD frame buffer.
	
	uint32_t width;		// Width of the image.
	uint32_t height;	// Height of the image.
	
	uint32_t pitch;		// Pitch. (bytes per scanline)
	uint32_t bpp;		// Bits per pixel.
} mdp_event_post_frame_t;


#ifdef __cplusplus
}
#endif

#endif /* __MDP_EVENT_H */
