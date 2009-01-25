/***************************************************************************
 * MDP: Mega Drive Plugin - Host Services.                                 *
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

#ifndef MDP_HOST_H
#define MDP_HOST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "mdp_fncall.h"
#include "mdp_version.h"
#include "mdp_render.h"
#include "mdp_event.h"

// MDP Host Services interface version.
#define MDP_HOST_INTERFACE_VERSION MDP_VERSION(0, 2, 0)

// MDP_PTR: Pointer IDs.
typedef enum
{
	MDP_PTR_NULL		= 0,
	MDP_PTR_LUT16to32	= 1,
	MDP_PTR_RGB16toYUV	= 2,
	MDP_PTR_ROM_MD		= 3, /* BE */
	MDP_PTR_ROM_32X		= 4, /* LE */
	MDP_PTR_RAM_MD		= 5, /* BE */
	MDP_PTR_RAM_VRAM	= 6,
	MDP_PTR_RAM_Z80		= 7,
	MDP_PTR_RAM_MCD_PRG	= 8, /* unknown */
	MDP_PTR_RAM_MCD_WORD1M	= 9, /* unknown */
	MDP_PTR_RAM_MCD_WORD2M	= 10, /* unknown */
	MDP_PTR_RAM_32X		= 11, /* LE */
} MDP_PTR;

// Convenience macros to access 8-bit and 16-bit memory.

#define MDP_MEM_16(ptr, address)	\
	(((unsigned short*)ptr)[address >> 1])

#if MDP_BYTEORDER == MDP_LIL_ENDIAN

// Litle-endian.
#define MDP_MEM_BE_8(ptr, address)	\
	(((unsigned char*)ptr)[address ^ 1])
#define MDP_MEM_LE_8(ptr, address)	\
	(((unsigned char*)ptr)[address])

#else

// Big-endian.
#define MDP_MEM_BE_8(ptr, address)	\
	(((unsigned char*)ptr)[address])
#define MDP_MEM_LE_8(ptr, address)	\
	(((unsigned char*)ptr)[address ^ 1])

#endif

// MDP_VAL: Value IDs.
typedef enum
{
	MDP_VAL_UI		= 0,
	MDP_VAL_ROM_SIZE	= 1,
} MDP_VAL;

// MDP_UI: UI identifiers.
typedef enum
{
	MDP_UI_NONE		= 0,
	MDP_UI_GTK2		= 1,
	MDP_UI_QT4		= 2,
	MDP_UI_WIN32		= 3,
	MDP_UI_MACOSX_COCOA	= 4,
} MDP_UI;

// MDP_MENU: Menu IDs.
typedef enum
{
	MDP_MENU_DEFAULT	= 0,
} MDP_MENU;

// Menu Handler function.
typedef int (MDP_FNCALL *mdp_menu_handler_fn)(int menu_item_id);

// MDP Host Services struct.
typedef struct
{
	const uint32_t interfaceVersion;
	
	// ptr_ref(): Get a reference for a pointer.
	// ptr_unref(): Unreference a pointer.
	void* (MDP_FNCALL *ptr_ref)(uint32_t ptrID);
	int   (MDP_FNCALL *ptr_unref)(uint32_t ptrID);
	
	// val_set(), val_get(): Set or get int values.
	int (MDP_FNCALL *val_set)(uint32_t valID, int val);
	int (MDP_FNCALL *val_get)(uint32_t valID);
	
	// renderer_register(): Register a renderer.
	// renderer_unregister(): Unregister a renderer.
	int (MDP_FNCALL *renderer_register)(struct MDP_t *plugin, MDP_Render_t *renderer);
	int (MDP_FNCALL *renderer_unregister)(struct MDP_t *plugin, MDP_Render_t *renderer);
	
	// menu_item_add(): Add a menu item.
	// menu_item_remove(): Remove a menu item.
	// menu_item_set_text(): Set menu item text.
	// menu_item_get_text(): Get menu item text.
	// menu_item_set_checked(): Set menu item "checked" state.
	// menu_item_get_checked(): Get menu item "checked" state.
	int (MDP_FNCALL *menu_item_add)(struct MDP_t *plugin, mdp_menu_handler_fn handler, int menu_id, const char *text);
	int (MDP_FNCALL *menu_item_remove)(struct MDP_t *plugin, int menu_item_id);
	int (MDP_FNCALL *menu_item_set_text)(struct MDP_t *plugin, int menu_item_id, const char *text);
	int (MDP_FNCALL *menu_item_get_text)(struct MDP_t *plugin, int menu_item_id, char *text_buf, int size);
	int (MDP_FNCALL *menu_item_set_checked)(struct MDP_t *plugin, int menu_item_id, int checked);
	int (MDP_FNCALL *menu_item_get_checked)(struct MDP_t *plugin, int menu_item_id);
	
	// Event handler functions.
	int (MDP_FNCALL *event_register)(struct MDP_t *plugin, int event_id, mdp_event_handler_fn handler);
	int (MDP_FNCALL *event_unregister)(struct MDP_t *plugin, int event_id, mdp_event_handler_fn handler);
	
	// Window registration.
	int (MDP_FNCALL *window_register)(struct MDP_t *plugin, void *window);
	int (MDP_FNCALL *window_unregister)(struct MDP_t *plugin, void *window);
} MDP_Host_t;

#ifdef __cplusplus
}
#endif

#endif /* MDP_HOST_H */
