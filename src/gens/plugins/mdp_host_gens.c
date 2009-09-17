/***************************************************************************
 * Gens: MDP: Mega Drive Plugin - Host Services.                           *
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Main window.
#include "gens/gens_window.h"
#include "libgsft/gsft_unused.h"

// C includes.
#include <stdio.h>
#include <stdlib.h>

// MDP Host Services includes.
#include "mdp_host_gens.h"
#include "mdp_host_gens_cpp.hpp"
#include "mdp_host_gens_menu.hpp"
#include "mdp_host_gens_event.hpp"
#include "mdp_host_gens_mem.h"
#include "mdp_host_gens_emuctrl.hpp"
#include "mdp_host_gens_z.h"
#include "mdp_host_gens_config.hpp"
#include "mdp_host_gens_dir.hpp"

// Register functions.
#include "reg/reg_get.h"
#include "reg/reg_set.h"
#include "reg/reg_get_all.h"
#include "reg/reg_set_all.h"

// MDP includes.
#include "mdp/mdp_error.h"

// MD variables.
#include "gens_core/mem/mem_m68k.h"
#include "gens_core/vdp/vdp_rend.h"

// CPU flags.
#include "gens_core/misc/cpuflags.h"

// ROM information.
#include "util/file/rom.hpp"

// Text drawing.
#include "video/vdraw_text.hpp"

// Text drawing functions.
static int MDP_FNCALL mdp_host_osd_printf(const char *msg, ...);


mdp_host_t Gens_MDP_Host =
{
	.interfaceVersion	= MDP_INTERFACE_VERSION,
	
	.val_set		= mdp_host_val_set,
	.val_get		= mdp_host_val_get,
	
	.osd_printf		= mdp_host_osd_printf,
	
	.renderer_register	= mdp_host_renderer_register,
	.renderer_unregister	= mdp_host_renderer_unregister,
	
	.mem_read_8		= mdp_host_mem_read_8,
	.mem_read_16		= mdp_host_mem_read_16,
	.mem_read_32		= mdp_host_mem_read_32,
	
	.mem_write_8		= mdp_host_mem_write_8,
	.mem_write_16		= mdp_host_mem_write_16,
	.mem_write_32		= mdp_host_mem_write_32,
	
	.mem_read_block_8	= mdp_host_mem_read_block_8,
	.mem_read_block_16	= mdp_host_mem_read_block_16,
	.mem_read_block_32	= mdp_host_mem_read_block_32,
	
	.mem_write_block_8	= mdp_host_mem_write_block_8,
	.mem_write_block_16	= mdp_host_mem_write_block_16,
	.mem_write_block_32	= mdp_host_mem_write_block_32,
	
	.mem_size_get		= mdp_host_mem_size_get,
	.mem_size_set		= mdp_host_mem_size_set,
	
	.reg_get		= mdp_host_reg_get,
	.reg_set		= mdp_host_reg_set,
	.reg_get_all		= mdp_host_reg_get_all,
	.reg_set_all		= mdp_host_reg_set_all,
	
	.menu_item_add		= mdp_host_menu_item_add,
	.menu_item_remove	= mdp_host_menu_item_remove,
	
	.menu_item_set_text	= mdp_host_menu_item_set_text,
	.menu_item_get_text	= mdp_host_menu_item_get_text,
	
	.menu_item_set_checked	= mdp_host_menu_item_set_checked,
	.menu_item_get_checked	= mdp_host_menu_item_get_checked,
	
	.event_register		= mdp_host_event_register,
	.event_unregister	= mdp_host_event_unregister,
	
	.window_register	= mdp_host_window_register,
	.window_unregister	= mdp_host_window_unregister,
	.window_get_main	= mdp_host_window_get_main,
	
	.emulator_control	= mdp_host_emulator_control,
	
	.config_set		= mdp_host_config_set,
	.config_get		= mdp_host_config_get,
	
	.dir_get_default_save_path = mdp_host_dir_get_default_save_path,
	.dir_register		= mdp_host_dir_register,
	.dir_unregister		= mdp_host_dir_unregister,
	
	.crc32			= mdp_host_crc32,
	.z_open			= mdp_host_z_open,
	.z_get_file		= mdp_host_z_get_file,
	.z_close		= mdp_host_z_close
};


/**
 * mdp_host_set_val(): Set an MDP value.
 * @param plugin Plugin requesting the change.
 * @param valID Value ID.
 * @param val Value.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_host_val_set(mdp_t *plugin, uint32_t valID, int val)
{
	GSFT_UNUSED_PARAMETER(plugin);
	
	switch (valID)
	{
		case MDP_VAL_UI:
		case MDP_VAL_CPU_FLAGS:
			// Read-only values.
			return -MDP_ERR_VAL_READ_ONLY;
			break;
		
		case MDP_VAL_VDP_LAYER_OPTIONS:
			// VDP layer options.
			VDP_Layers = (unsigned int)val;
			break;
		
		default:
			// Unknown value ID.
			return -MDP_ERR_UNKNOWN_VALID;
			break;
	}
	
	return MDP_ERR_OK;
}


/**
 * mdp_host_set_val(): Get an MDP value.
 * @param valID Value ID.
 * @return Value, or MDP error code. (Values must be positive.)
 */
int MDP_FNCALL mdp_host_val_get(uint32_t valID)
{
	switch (valID)
	{
		case MDP_VAL_UI:
			// UI type.
			#if defined(GENS_UI_GTK)
				return MDP_UI_GTK2;
			#elif defined(GENS_UI_QT4)
				return MDP_UI_QT4;
			#elif defined(GENS_UI_WIN32)
				return MDP_UI_WIN32;
			#elif defined(GENS_UI_MACOSX_COCOA)
				return MDP_UI_MACOSX_COCOA;
			#else
				return MDP_UI_NONE;
			#endif
		
		case MDP_VAL_CPU_FLAGS:
			// CPU flags.
			return CPU_Flags;
		
		case MDP_VAL_VDP_LAYER_OPTIONS:
			// VDP layer options.
			return VDP_Layers;
		
		default:
			// Unknown value ID.
			return -MDP_ERR_UNKNOWN_VALID;
	}
	
	return MDP_ERR_OK;
}


/**
 * mdp_host_window_get_main(): Get a pointer to the main window.
 * @return Pointer to the main window.
 */
void* MDP_FNCALL mdp_host_window_get_main(void)
{
	return gens_window;
}


/**
 * mdp_host_osd_printf(): Print formatted text to the screen. (va_list version)
 * @param msg Message to write. (printf-formatted)
 * @param ap Format arguments.
 * @return MDP error code.
 */
static int MDP_FNCALL mdp_host_osd_printf(const char *msg, ...)
{
	if (!msg)
		return -MDP_ERR_INVALID_PARAMETERS;
	
	// Print the message.
	va_list ap;
	va_start(ap, msg);
	vdraw_text_vprintf(1500, msg, ap);
	va_end(ap);
	
	return MDP_ERR_OK;
}
