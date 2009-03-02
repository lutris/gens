/***************************************************************************
 * MDP: Mega Drive Plugins - Host Services.                                *
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

#ifndef __MDP_HOST_H
#define __MDP_HOST_H

#include <stdint.h>

#include "mdp_fncall.h"
#include "mdp_version.h"
#include "mdp_render.h"
#include "mdp_event.h"

#ifdef __cplusplus
extern "C" {
#endif

/* MDP Host Services interface version. */
#define MDP_HOST_INTERFACE_VERSION MDP_VERSION(0, 2, 0)

/* MDP_PTR: Pointer IDs. */
typedef enum
{
	MDP_PTR_NULL		= 0,
	MDP_PTR_LUT16to32	= 1,
	MDP_PTR_ROM_MD		= 3, /* BE */
	MDP_PTR_ROM_32X		= 4, /* LE */
	MDP_PTR_RAM_MD		= 5, /* BE */
	MDP_PTR_RAM_VRAM	= 6, /* BE */
	MDP_PTR_RAM_Z80		= 7,
	MDP_PTR_RAM_MCD_PRG	= 8, /* unknown */
	MDP_PTR_RAM_MCD_WORD1M	= 9, /* unknown */
	MDP_PTR_RAM_MCD_WORD2M	= 10, /* unknown */
	MDP_PTR_RAM_32X		= 11, /* LE */
} MDP_PTR;

/* Convenience macros to access 8-bit, 16-bit, and 32-bit memory. */

#define MDP_MEM_16(ptr, address)	\
	(((unsigned short*)ptr)[(address) >> 1])

#define MDP_MEM_BE_32_READ(ptr, address)	\
	(((((unsigned short*)(ptr))[(address) >> 1]) << 16) | (((unsigned short*)ptr)[((address) >> 1) + 1]))
#define MDP_MEM_BE_32_WRITE(ptr, address, data)	\
	((unsigned short*)(ptr))[(address) >> 1] = (((data) >> 16) & 0xFFFF);	\
	((unsigned short*)(ptr))[((address) >> 1) + 1] = ((data) & 0xFFFF);

#define MDP_MEM_LE_32_READ(ptr, address)	\
	(((((unsigned short*)(ptr))[((address) >> 1) + 1]) << 16) | (((unsigned short*)ptr)[(address) >> 1]))
#define MDP_MEM_LE_32_WRITE(ptr, address, data)	\
	((unsigned short*)(ptr))[((address) >> 1) + 1] = (((data) >> 16) & 0xFFFF);	\
	((unsigned short*)(ptr))[(address) >> 1] = ((data) & 0xFFFF);

#if MDP_BYTEORDER == MDP_LIL_ENDIAN

/* Little-endian host system. */
#define MDP_MEM_BE_8(ptr, address)	\
	(((unsigned char*)(ptr))[(address) ^ 1])
#define MDP_MEM_LE_8(ptr, address)	\
	(((unsigned char*)(ptr))[(address)])

#else

/* Big-endian host system. */
#define MDP_MEM_BE_8(ptr, address)	\
	(((unsigned char*)(ptr))[(address)])
#define MDP_MEM_LE_8(ptr, address)	\
	(((unsigned char*)(ptr))[(address) ^ 1])

#endif

/* MDP_VAL: Value IDs. */
typedef enum _MDP_VAL
{
	MDP_VAL_UI			= 0,
	MDP_VAL_ROM_SIZE		= 1,
	MDP_VAL_VDP_LAYER_OPTIONS	= 2,
} MDP_VAL;

/* MDP_UI: UI identifiers. */
typedef enum _MDP_UI
{
	MDP_UI_NONE		= 0,
	MDP_UI_GTK2		= 1,
	MDP_UI_QT4		= 2,
	MDP_UI_WIN32		= 3,
	MDP_UI_MACOSX_COCOA	= 4,
} MDP_UI;

/* MDP_VDP_LAYER_OPTIONS: Layer options bits. */
#define MDP_VDP_LAYER_OPTIONS_SCROLLA_LOW		(1 << 0)
#define MDP_VDP_LAYER_OPTIONS_SCROLLA_HIGH		(1 << 1)
#define MDP_VDP_LAYER_OPTIONS_SCROLLA_SWAP		(1 << 2)
#define MDP_VDP_LAYER_OPTIONS_SCROLLB_LOW		(1 << 3)
#define MDP_VDP_LAYER_OPTIONS_SCROLLB_HIGH		(1 << 4)
#define MDP_VDP_LAYER_OPTIONS_SCROLLB_SWAP		(1 << 5)
#define MDP_VDP_LAYER_OPTIONS_SPRITE_LOW		(1 << 6)
#define MDP_VDP_LAYER_OPTIONS_SPRITE_HIGH		(1 << 7)
#define MDP_VDP_LAYER_OPTIONS_SPRITE_SWAP		(1 << 8)
#define MDP_VDP_LAYER_OPTIONS_SPRITE_ALWAYSONTOP	(1 << 9)
#define	MDP_VDP_LAYER_OPTIONS_PALETTE_LOCK		(1 << 10)

#define MDP_VDP_LAYER_OPTIONS_DEFAULT	  \
	(MDP_VDP_LAYER_OPTIONS_SCROLLA_LOW	| \
	 MDP_VDP_LAYER_OPTIONS_SCROLLA_HIGH	| \
	 MDP_VDP_LAYER_OPTIONS_SCROLLB_LOW	| \
	 MDP_VDP_LAYER_OPTIONS_SCROLLB_HIGH	| \
	 MDP_VDP_LAYER_OPTIONS_SPRITE_LOW	| \
	 MDP_VDP_LAYER_OPTIONS_SPRITE_HIGH)

// MDP_MENU: Menu IDs.
typedef enum _MDP_MENU
{
	MDP_MENU_DEFAULT	= 0,
} MDP_MENU;

// Menu Handler function.
typedef int (MDP_FNCALL *mdp_menu_handler_fn)(int menu_item_id);

// MDP Host Services struct.
typedef struct _mdp_host_t
{
	const uint32_t interfaceVersion;
	
	/**
	 * ptr_ref(): Get a reference for a pointer.
	 * ptr_unref(): Unreference a pointer.
	 */
	void* (MDP_FNCALL *ptr_ref)(uint32_t ptrID);
	int   (MDP_FNCALL *ptr_unref)(uint32_t ptrID);
	
	/**
	 * val_set(), val_get(): Set or get int values.
	 */
	int (MDP_FNCALL *val_set)(uint32_t valID, int val);
	int (MDP_FNCALL *val_get)(uint32_t valID);
	
	/**
	 * renderer_register(): Register a renderer.
	 * renderer_unregister(): Unregister a renderer.
	 */
	int (MDP_FNCALL *renderer_register)(struct _mdp_t *plugin, mdp_render_t *renderer);
	int (MDP_FNCALL *renderer_unregister)(struct _mdp_t *plugin, mdp_render_t *renderer);
	
	/**
	 * menu_item_add(): Add a menu item.
	 * menu_item_remove(): Remove a menu item.
	 * menu_item_set_text(): Set menu item text.
	 * menu_item_get_text(): Get menu item text.
	 * menu_item_set_checked(): Set menu item "checked" state.
	 * menu_item_get_checked(): Get menu item "checked" state.
	 */
	int (MDP_FNCALL *menu_item_add)(struct _mdp_t *plugin, mdp_menu_handler_fn handler, int menu_id, const char *text);
	int (MDP_FNCALL *menu_item_remove)(struct _mdp_t *plugin, int menu_item_id);
	int (MDP_FNCALL *menu_item_set_text)(struct _mdp_t *plugin, int menu_item_id, const char *text);
	int (MDP_FNCALL *menu_item_get_text)(struct _mdp_t *plugin, int menu_item_id, char *text_buf, int size);
	int (MDP_FNCALL *menu_item_set_checked)(struct _mdp_t *plugin, int menu_item_id, int checked);
	int (MDP_FNCALL *menu_item_get_checked)(struct _mdp_t *plugin, int menu_item_id);
	
	/* Event handler functions. */
	int (MDP_FNCALL *event_register)(struct _mdp_t *plugin, int event_id, mdp_event_handler_fn handler);
	int (MDP_FNCALL *event_unregister)(struct _mdp_t *plugin, int event_id, mdp_event_handler_fn handler);
	
	/* Window registration. */
	int (MDP_FNCALL *window_register)(struct _mdp_t *plugin, void *window);
	int (MDP_FNCALL *window_unregister)(struct _mdp_t *plugin, void *window);
	void* (MDP_FNCALL *window_get_main)(void);
	
	/* Directory functions. */
	int (MDP_FNCALL *directory_get_default_save_path)(char* buf, int size);
} mdp_host_t;

#ifdef __cplusplus
}
#endif

#endif /* __MDP_HOST_H */
