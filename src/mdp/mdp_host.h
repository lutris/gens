/***************************************************************************
 * MDP: Mega Drive Plugins - Host Services.                                *
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

#ifndef __MDP_HOST_H
#define __MDP_HOST_H

#include "mdp_fncall.h"
#include "mdp_version.h"
#include "mdp_stdint.h"

#include "mdp_render.h"
#include "mdp_event.h"
#include "mdp_z.h"

#ifdef __cplusplus
extern "C" {
#endif

/* MDP_VAL: Value IDs. */
typedef enum _MDP_VAL
{
	/*! BEGIN: MDP v1.0 Value IDs. !*/
	MDP_VAL_UI			= 0,
	MDP_VAL_CPU_FLAGS		= 1,
	MDP_VAL_VDP_LAYER_OPTIONS	= 2,
	/*! END: MDP v1.0 Value IDs. !*/
	
	MDP_VAL_MAX
} MDP_VAL;

/* MDP_UI: UI identifiers. */
typedef enum _MDP_UI
{
	/*! BEGIN: MDP v1.0 UI identifiers. !*/
	MDP_UI_NONE		= 0,
	MDP_UI_GTK2		= 1,
	MDP_UI_QT4		= 2,
	MDP_UI_WIN32		= 3,
	MDP_UI_MACOSX_COCOA	= 4,
	MDP_UI_HAIKU		= 5,
	/*! END: MDP v1.0 UI identifiers. !*/
	
	MDP_UI_MAX
} MDP_UI;

/** MDP_VDP_LAYER_OPTIONS: Layer options bits. **/
/*! BEGIN: MDP v1.0 VDP layer options bits. !*/
#define MDP_VDP_LAYER_OPTIONS_SCROLLA_LOW		((uint32_t)(1 << 0))
#define MDP_VDP_LAYER_OPTIONS_SCROLLA_HIGH		((uint32_t)(1 << 1))
#define MDP_VDP_LAYER_OPTIONS_SCROLLA_SWAP		((uint32_t)(1 << 2))
#define MDP_VDP_LAYER_OPTIONS_SCROLLB_LOW		((uint32_t)(1 << 3))
#define MDP_VDP_LAYER_OPTIONS_SCROLLB_HIGH		((uint32_t)(1 << 4))
#define MDP_VDP_LAYER_OPTIONS_SCROLLB_SWAP		((uint32_t)(1 << 5))
#define MDP_VDP_LAYER_OPTIONS_SPRITE_LOW		((uint32_t)(1 << 6))
#define MDP_VDP_LAYER_OPTIONS_SPRITE_HIGH		((uint32_t)(1 << 7))
#define MDP_VDP_LAYER_OPTIONS_SPRITE_SWAP		((uint32_t)(1 << 8))
#define MDP_VDP_LAYER_OPTIONS_SPRITE_ALWAYSONTOP	((uint32_t)(1 << 9))
#define	MDP_VDP_LAYER_OPTIONS_PALETTE_LOCK		((uint32_t)(1 << 10))

#define MDP_VDP_LAYER_OPTIONS_DEFAULT	  \
	(MDP_VDP_LAYER_OPTIONS_SCROLLA_LOW	| \
	 MDP_VDP_LAYER_OPTIONS_SCROLLA_HIGH	| \
	 MDP_VDP_LAYER_OPTIONS_SCROLLB_LOW	| \
	 MDP_VDP_LAYER_OPTIONS_SCROLLB_HIGH	| \
	 MDP_VDP_LAYER_OPTIONS_SPRITE_LOW	| \
	 MDP_VDP_LAYER_OPTIONS_SPRITE_HIGH)
/*! END: MDP v1.0 VDP layer options bits. !*/

/* MDP_MENU: Menu IDs. */
typedef enum _MDP_MENU
{
	/*! BEGIN: MDP v1.0 menu IDs. !*/
	MDP_MENU_DEFAULT	= 0,
	MDP_MENU_PLUGINS	= 0,
	MDP_MENU_VIDEO		= 1,
	MDP_MENU_AUDIO		= 2,
	/*! END: MDP v1.0 menu IDs. !*/
	
	MDP_MENU_MAX
} MDP_MENU;

/* Menu Handler function. */
typedef int (MDP_FNCALL *mdp_menu_handler_fn)(int menu_item_id);

/** MDP_EMUCTRL: MDP Emulator Control functions. **/
typedef enum _MDP_EMUCTRL
{
	/*! BEGIN: MDP v1.0 emulator control functions. !*/
	MDP_EMUCTRL_UNKNOWN		= 0,
	MDP_EMUCTRL_RESET_HARD		= 1,	/* Reset emulation. (Hard Reset) */
	MDP_EMUCTRL_RESET_SOFT		= 2,	/* Reset emulation. (Soft Reset) */
	MDP_EMUCTRL_RELOAD_INFO		= 3,	/* Reload ROM information. */
	/*! END: MDP v1.0 emulator control functions. !*/
	
	MDP_EMUCTRL_MAX
} MDP_EMUCTRL;

/* Directory Handler functions. */
typedef int (MDP_FNCALL *mdp_dir_get_fn)(int dir_id, char *out_buf, unsigned int size);
typedef int (MDP_FNCALL *mdp_dir_set_fn)(int dir_id, const char *buf);

/** MDP Host Services struct. **/
typedef struct _mdp_host_t
{
	/**
	 * interfaceVersion: MDP version implemented by the emulator.
	 */
	const uint32_t interfaceVersion;
	const uint32_t reserved;
	
	/*! BEGIN: MDP v1.0 host services functions. !*/
	
	/**
	 * val_set(), val_get(): Set or get int values.
	 */
	int (MDP_FNCALL *val_set)(struct _mdp_t *plugin, uint32_t valID, int val);
	int (MDP_FNCALL *val_get)(uint32_t valID);
	
	/**
	 * osd_printf(): Print a message to the on-screen display.
	 */
	int (MDP_FNCALL *osd_printf)(const char *msg, ...)
		__attribute__ ((format (printf, 1, 2)));
	
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
	 * @param ret_value Variable to store the return value in.
	 * @return MDP error code.
	 */
	int (MDP_FNCALL *mem_read_8) (int memID, uint32_t address, uint8_t  *ret_value);
	int (MDP_FNCALL *mem_read_16)(int memID, uint32_t address, uint16_t *ret_value);
	int (MDP_FNCALL *mem_read_32)(int memID, uint32_t address, uint32_t *ret_value);
	
	/**
	 * mem_write_*(): Memory write functions.
	 * @param plugin Plugin requesting memory write.
	 * @param memID Memory ID.
	 * @param address Address.
	 * @param data Data.
	 * @return MDP error code.
	 */
	int (MDP_FNCALL *mem_write_8) (struct _mdp_t *plugin, int memID, uint32_t address, uint8_t  data);
	int (MDP_FNCALL *mem_write_16)(struct _mdp_t *plugin, int memID, uint32_t address, uint16_t data);
	int (MDP_FNCALL *mem_write_32)(struct _mdp_t *plugin, int memID, uint32_t address, uint32_t data);
	
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
	 * @param plugin Plugin requesting memory write.
	 * @param memID Memory ID.
	 * @param address Starting address.
	 * @param data Data buffer containing the data to write.
	 * @param length Length of the data.
	 * @return MDP error code.
	 */
	int (MDP_FNCALL *mem_write_block_8) (struct _mdp_t *plugin, int memID, uint32_t address, uint8_t  *data, uint32_t length);
	int (MDP_FNCALL *mem_write_block_16)(struct _mdp_t *plugin, int memID, uint32_t address, uint16_t *data, uint32_t length);
	int (MDP_FNCALL *mem_write_block_32)(struct _mdp_t *plugin, int memID, uint32_t address, uint32_t *data, uint32_t length);
	
	/**
	 * mem_size_get(): Get the size of a memory block.
	 * @param memID Memory ID.
	 * @return Memory size, or negative on error.
	 */
	int (MDP_FNCALL *mem_size_get)(int memID);
	
	/**
	 * mem_size_set(): Set the size of a memory block.
	 * @param plugin Plugin requesting memory size set.
	 * @param memID Memory ID.
	 * @param size New memory size.
	 * @return MDP error code.
	 */
	int (MDP_FNCALL *mem_size_set)(struct _mdp_t *plugin, int memID, unsigned int size);
	
	/**
	 * reg_get(): Get a register value.
	 * @param icID ID of the IC to get a register value from.
	 * @param regID Register ID.
	 * @param ret_value Variable to store the return value in.
	 * @return MDP error code.
	 */
	int (MDP_FNCALL *reg_get)(int icID, int regID, uint32_t *ret_value);
	
	/**
	 * reg_set(): Get a register value.
	 * @param plugin Plugin requesting register set.
	 * @param icID ID of the IC to set a register value in.
	 * @param regID Register ID.
	 * @param new_value New register value.
	 * @return MDP error code.
	 */
	int (MDP_FNCALL *reg_set)(struct _mdp_t *plugin, int icID, int regID, uint32_t new_value);
	
	/**
	 * reg_get_all(): Get all register values for a given IC.
	 * @param icID ID of the IC to get all register values from.
	 * @param reg_struct Pointer to register struct for this IC.
	 * @return MDP error code.
	 */
	int (MDP_FNCALL *reg_get_all)(int icID, void *reg_struct);
	
	/**
	 * reg_set_all(): Set all register values for a given IC.
	 * @param plugin Plugin requesting register set.
	 * @param icID ID of the IC to set all register values in.
	 * @param reg_struct Pointer to register struct for this IC.
	 * @return MDP error code.
	 */
	int (MDP_FNCALL *reg_set_all)(struct _mdp_t *plugin, int icID, void *reg_struct);
	
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
	int (MDP_FNCALL *menu_item_get_text)(struct _mdp_t *plugin, int menu_item_id, char *text_buf, unsigned int size);
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
	
	/* Configuration functions. */
	int (MDP_FNCALL *config_get)(struct _mdp_t *plugin, const char* key, const char* def, char *out_buf, unsigned int size);
	int (MDP_FNCALL *config_set)(struct _mdp_t *plugin, const char* key, const char* value);
	
	/* Directory functions. */
	int (MDP_FNCALL *dir_get_default_save_path)(char* buf, unsigned int size);
	int (MDP_FNCALL *dir_register)(struct _mdp_t *plugin, const char *dir_name,
				       mdp_dir_get_fn get_fn, mdp_dir_set_fn set_fn);
	int (MDP_FNCALL *dir_unregister)(struct _mdp_t *plugin, int dir_id);
	
	/* Compression functions. */
	int (MDP_FNCALL *crc32)(const uint8_t* buf, int length, uint32_t *crc32_out);
	int (MDP_FNCALL *z_open)(const char* filename, mdp_z_t **z_out);
	int (MDP_FNCALL *z_get_file)(mdp_z_t *z_file, mdp_z_entry_t *z_entry, void *buf, size_t size);
	int (MDP_FNCALL *z_close)(mdp_z_t *z_file);
	
	/*! END: MDP v1.0 host services functions. !*/
} mdp_host_t;

#ifdef __cplusplus
}
#endif

#endif /* __MDP_HOST_H */
