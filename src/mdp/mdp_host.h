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
#include "mdp_z.h"

#ifdef __cplusplus
extern "C" {
#endif

/* MDP Host Services interface version. */
#define MDP_HOST_INTERFACE_VERSION MDP_VERSION(0, 2, 0)

/* MDP_PTR: Pointer IDs. */
typedef enum _MDP_PTR
{
	MDP_PTR_NULL		= 0,
	MDP_PTR_LUT16to32	= 1,
} MDP_PTR;

/* MDP_MEM: Memory IDs. */
typedef enum _MDP_MEM
{
	MDP_MEM_NULL		= 0,
	MDP_MEM_MD_ROM		= 1,
	MDP_MEM_MD_RAM		= 2,
	MDP_MEM_MD_VRAM		= 3,
	#if 0 /* TODO: Figure out how to manage these. */
	MDP_MEM_Z80_RAM		= 4,
	MDP_MEM_MCD_PRG_RAM	= 5,
	MDP_MEM_MCD_WORD_RAM	= 6,
	MDP_MEM_32X_RAM		= 7,
	#endif
} MDP_MEM;

/* MDP_VAL: Value IDs. */
typedef enum _MDP_VAL
{
	MDP_VAL_UI			= 0,
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

/* MDP_MENU: Menu IDs. */
typedef enum _MDP_MENU
{
	MDP_MENU_DEFAULT	= 0,
} MDP_MENU;

/* Menu Handler function. */
typedef int (MDP_FNCALL *mdp_menu_handler_fn)(int menu_item_id);

/* MDP Emulator Control functions. */
typedef enum _MDP_EMUCTRL
{
	MDP_EMUCTRL_UNKNOWN		= 0,
	MDP_EMUCTRL_RESET		= 1,	/* Reset emulation. */
	MDP_EMUCTRL_RELOAD_INFO		= 2,	/* Reload ROM information. */
} MDP_EMUCTRL;

/* MDP Host Services struct. */
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
	 * mem_read_*(): Memory read functions.
	 * @param memID Memory ID.
	 * @param address Address.
	 * @return Data.
	 */
	uint8_t  (MDP_FNCALL *mem_read_8) (int memID, uint32_t address);
	uint16_t (MDP_FNCALL *mem_read_16)(int memID, uint32_t address);
	uint32_t (MDP_FNCALL *mem_read_32)(int memID, uint32_t address);
	
	/**
	 * mem_write_*(): Memory write functions.
	 * @param memID Memory ID.
	 * @param address Address.
	 * @param data Data.
	 * @return MDP error code.
	 */
	int (MDP_FNCALL *mem_write_8) (int memID, uint32_t address, uint8_t  data);
	int (MDP_FNCALL *mem_write_16)(int memID, uint32_t address, uint16_t data);
	int (MDP_FNCALL *mem_write_32)(int memID, uint32_t address, uint32_t data);
	
	/**
	 * mem_read_block_*: Memory block read functions.
	 * @param memID Memory ID.
	 * @param address Starting address.
	 * @param data Data buffer to store the data in.
	 * @param length Length of the data.
	 * @return MDP error code.
	 */
	int (MDP_FNCALL *mem_read_block_8) (int memID, uint32_t address, uint8_t  *data, uint32_t length);
	int (MDP_FNCALL *mem_read_block_16)(int memID, uint32_t address, uint16_t *data, uint32_t length);
	int (MDP_FNCALL *mem_read_block_32)(int memID, uint32_t address, uint32_t *data, uint32_t length);
	
	/**
	 * mem_write_block_*: Memory block write functions.
	 * @param memID Memory ID.
	 * @param address Starting address.
	 * @param data Data buffer containing the data to write.
	 * @param length Length of the data.
	 * @return MDP error code.
	 */
	int (MDP_FNCALL *mem_write_block_8) (int memID, uint32_t address, uint8_t  *data, uint32_t length);
	int (MDP_FNCALL *mem_write_block_16)(int memID, uint32_t address, uint16_t *data, uint32_t length);
	int (MDP_FNCALL *mem_write_block_32)(int memID, uint32_t address, uint32_t *data, uint32_t length);
	
	/**
	 * mem_size_get(): Get the size of a memory block.
	 * @param memID Memory ID.
	 * @return Memory size, or negative on error.
	 */
	int (MDP_FNCALL *mem_size_get)(int memID);
	
	/**
	 * mem_size_set(): Set the size of a memory block.
	 * @param memID Memory ID.
	 * @param size New memory size.
	 * @return MDP error code.
	 */
	int (MDP_FNCALL *mem_size_set)(int memID, int size);
	
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
	
	/* Emulator control. */
	int (MDP_FNCALL *emulator_control)(struct _mdp_t *plugin, MDP_EMUCTRL ctrl, void *param);
	
	/* Directory functions. */
	int (MDP_FNCALL *directory_get_default_save_path)(char* buf, int size);
	
	/* Compression functions. */
	uint32_t	(MDP_FNCALL *crc32)(const uint8_t* buf, int length);
	mdp_z_t*	(MDP_FNCALL *z_open)(const char* filename);
	int		(MDP_FNCALL *z_get_file)(mdp_z_t *z_file, mdp_z_entry_t *z_entry, void *buf, size_t size);
	int		(MDP_FNCALL *z_close)(mdp_z_t *z_file);
} mdp_host_t;

#ifdef __cplusplus
}
#endif

#endif /* __MDP_HOST_H */
