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

// Main emulation functions.
#include "emulator/g_main.hpp"

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

// MDP includes.
#include "mdp/mdp_error.h"

// MD variables.
#include "gens_core/mem/mem_m68k.h"
#include "gens_core/vdp/vdp_rend.h"

// CPU flags.
#include "gens_core/misc/cpuflags.h"

// ROM information.
#include "util/file/rom.hpp"

// MDP_PTR functions.
static inline int* mdp_host_ptr_ref_LUT16to32(void);
static inline void mdp_host_ptr_unref_LUT16to32(void);

// MDP_PTR variables.
static int* mdp_ptr_LUT16to32 = NULL;
static int  mdp_ptr_LUT16to32_count = 0;


mdp_host_t Gens_MDP_Host =
{
	.interfaceVersion	= MDP_HOST_INTERFACE_VERSION,
	
	.ptr_ref		= mdp_host_ptr_ref,
	.ptr_unref		= mdp_host_ptr_unref,
	
	.val_set		= mdp_host_val_set,
	.val_get		= mdp_host_val_get,
	
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
	
	.crc32			= mdp_host_crc32,
	.z_open			= mdp_host_z_open,
	.z_get_file		= mdp_host_z_get_file,
	.z_close		= mdp_host_z_close
};


/**
 * mdp_host_ptr_ref(): Reference a pointer.
 * @param ptrID Pointer ID.
 * @return Pointer.
 */
void* MDP_FNCALL mdp_host_ptr_ref(uint32_t ptrID)
{
	switch (ptrID)
	{
		case MDP_PTR_LUT16to32:
			return (void*)mdp_host_ptr_ref_LUT16to32();
		default:
			fprintf(stderr, "%s: Invalid ptrID: 0x%08X\n", __func__, ptrID);
			return NULL;
	}
}

/**
 * mdp_host_ptr_unref(): Unreference a pointer.
 * @param ptrID Pointer ID.
 * @return Error code.
 */
int MDP_FNCALL mdp_host_ptr_unref(uint32_t ptrID)
{
	switch (ptrID)
	{
		case MDP_PTR_LUT16to32:
			mdp_host_ptr_unref_LUT16to32();
			break;
		default:
			fprintf(stderr, "%s: Unknown ptrID: 0x%08X\n", __func__, ptrID);
			return -MDP_ERR_UNKNOWN_PTRID;
			break;
	}
	
	return MDP_ERR_OK;
}


/**
 * mdp_host_ptr_ref_LUT16to32(): Get a reference for LUT16to32.
 * @return LUT16to32.
 */
static inline int* mdp_host_ptr_ref_LUT16to32(void)
{
	if (!mdp_ptr_LUT16to32)
	{
		// Allocate memory for the lookup table.
		mdp_ptr_LUT16to32 = (int*)(malloc(65536 * sizeof(int)));
		
		// Initialize the 16-bit to 32-bit conversion table.
		int i;
		for (i = 0; i < 65536; i++)
			mdp_ptr_LUT16to32[i] = ((i & 0xF800) << 8) + ((i & 0x07E0) << 5) + ((i & 0x001F) << 3);
	}
	
	// Increment the reference counter.
	mdp_ptr_LUT16to32_count++;
	
	// Return the pointer.
	return mdp_ptr_LUT16to32;
}

/**
 * mdp_host_ptr_unref_LUT16to32(): Unreference LUT16to32.
 * @return LUT16to32.
 */
static inline void mdp_host_ptr_unref_LUT16to32(void)
{
	// Decrement the reference counter.
	mdp_ptr_LUT16to32_count--;
	
	if (mdp_ptr_LUT16to32_count <= 0)
	{
		// All references are gone. Free the lookup table.
		free(mdp_ptr_LUT16to32);
		mdp_ptr_LUT16to32 = NULL;
	}
}


/**
 * mdp_host_set_val(): Set an MDP value.
 * @param valID Value ID.
 * @param val Value.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_host_val_set(uint32_t valID, int val)
{
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
 * mdp_host_dir_get_default_save_path(): Get the default save path.
 * @param buf Buffer to store the default save path in.
 * @param size Size of the buffer.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_host_dir_get_default_save_path(char *buf, unsigned int size)
{
	if (!buf || !size)
	{
		// Invalid parameters.
		return -MDP_ERR_INVALID_PARAMETERS;
	}
	
	// TODO: Return an error if the buffer is too small.
	#ifdef GENS_OS_WIN32
		// Win32's default save path is ".\\".
		// Return the full save path instead.
		strncpy(buf, PathNames.Gens_EXE_Path, size);
	#else
		// Get the actual default save path.
		get_default_save_path(buf, size);
	#endif
	
	// Make sure the return buffer is null-terminated.
	buf[size-1] = 0x00;
	
	return MDP_ERR_OK;
}
