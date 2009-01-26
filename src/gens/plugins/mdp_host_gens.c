/***************************************************************************
 * Gens: MDP: Mega Drive Plugin - Host Services.                           *
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

// Main window.
#include "gens/gens_window.hpp"

// C includes.
#include <stdio.h>
#include <stdlib.h>

// MDP Host Services includes.
#include "mdp_host_gens.h"
#include "mdp_host_gens_cpp.hpp"
#include "mdp_host_gens_menu.hpp"
#include "mdp_host_gens_event.hpp"

// MDP includes.
#include "mdp/mdp_error.h"

// MD variables.
#include "gens_core/mem/mem_m68k.h"
#include "gens_core/mem/mem_sh2.h"
#include "gens_core/vdp/vdp_io.h"
#include "gens_core/mem/mem_z80.h"
#include "gens_core/mem/mem_s68k.h"

// ROM information.
#include "util/file/rom.hpp"

// MDP_PTR functions.
static inline int* mdp_host_ptr_ref_LUT16to32(void);
static inline void mdp_host_ptr_unref_LUT16to32(void);
static inline int* mdp_host_ptr_ref_RGB16toYUV(void);
static inline void mdp_host_ptr_unref_RGB16toYUV(void);

// MDP_PTR variables.
static int* mdp_ptr_LUT16to32 = NULL;
static int  mdp_ptr_LUT16to32_count = 0;
static int* mdp_ptr_RGB16toYUV = NULL;
static int  mdp_ptr_RGB16toYUV_count = 0;


MDP_Host_t Gens_MDP_Host =
{
	.interfaceVersion = MDP_HOST_INTERFACE_VERSION,
	
	.ptr_ref = mdp_host_ptr_ref,
	.ptr_unref = mdp_host_ptr_unref,
	
	.val_set = mdp_host_val_set,
	.val_get = mdp_host_val_get,
	
	.renderer_register = mdp_host_renderer_register,
	.renderer_unregister = mdp_host_renderer_unregister,
	
	.menu_item_add = mdp_host_menu_item_add,
	.menu_item_remove = mdp_host_menu_item_remove,
	
	.menu_item_set_text = mdp_host_menu_item_set_text,
	.menu_item_get_text = mdp_host_menu_item_get_text,
	
	.menu_item_set_checked = mdp_host_menu_item_set_checked,
	.menu_item_get_checked = mdp_host_menu_item_get_checked,
	
	.event_register = mdp_host_event_register,
	.event_unregister = mdp_host_event_unregister,
	
	.window_register = mdp_host_window_register,
	.window_unregister = mdp_host_window_unregister,
	.window_get_main = mdp_host_window_get_main
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
		case MDP_PTR_RGB16toYUV:
			return (void*)mdp_host_ptr_ref_RGB16toYUV();
		case MDP_PTR_ROM_MD:
			return &Rom_Data;
		case MDP_PTR_ROM_32X:
			return &_32X_Rom;
		case MDP_PTR_RAM_MD:
			return &Ram_68k;
		case MDP_PTR_RAM_VRAM:
			return &VRam;
		case MDP_PTR_RAM_Z80:
			return &Ram_Z80;
		case MDP_PTR_RAM_MCD_PRG:
			return &Ram_Prg;
		case MDP_PTR_RAM_MCD_WORD1M:
			return &Ram_Word_1M;
		case MDP_PTR_RAM_MCD_WORD2M:
			return &Ram_Word_2M;
		case MDP_PTR_RAM_32X:
			return &_32X_Ram;
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
		
		case MDP_PTR_RGB16toYUV:
			mdp_host_ptr_unref_RGB16toYUV();
			break;
		
		case MDP_PTR_ROM_MD:
		case MDP_PTR_ROM_32X:
		case MDP_PTR_RAM_MD:
		case MDP_PTR_RAM_VRAM:
		case MDP_PTR_RAM_Z80:
		case MDP_PTR_RAM_MCD_PRG:
		case MDP_PTR_RAM_MCD_WORD1M:
		case MDP_PTR_RAM_MCD_WORD2M:
		case MDP_PTR_RAM_32X:
			// Nothing to unreference.
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
 * mdp_host_ptr_ref_LUT16to32(): Get a reference for RGB16toYUV.
 * @return RGB16toYUV.
 */
static inline int* mdp_host_ptr_ref_RGB16toYUV(void)
{
	if (!mdp_ptr_RGB16toYUV)
	{
		// Allocate memory for the lookup table.
		mdp_ptr_RGB16toYUV = (int*)(malloc(65536 * sizeof(int)));
		
		// Initialize the RGB to YUV conversion table.
		int i, j, k, r, g, b, Y, u, v;
		
		for (i = 0; i < 32; i++)
		{
			for (j = 0; j < 64; j++)
			{
				for (k = 0; k < 32; k++)
				{
					r = i << 3;
					g = j << 2;
					b = k << 3;
					Y = (r + g + b) >> 2;
					u = 128 + ((r - b) >> 2);
					v = 128 + ((-r + 2*g -b) >> 3);
					mdp_ptr_RGB16toYUV[(i << 11) + (j << 5) + k] = (Y << 16) + (u << 8) + v;
				}
			}
		}
	}
	
	// Increment the reference counter.
	mdp_ptr_RGB16toYUV_count++;
	
	// Return the pointer.
	return mdp_ptr_RGB16toYUV;
}

/**
 * mdp_host_ptr_unref_RGB16toYUV(): Unreference LUT16to32.
 * @return RGB16toYUV.
 */
static inline void mdp_host_ptr_unref_RGB16toYUV(void)
{
	// Decrement the reference counter.
	mdp_ptr_RGB16toYUV_count--;
	
	if (mdp_ptr_RGB16toYUV_count <= 0)
	{
		// All references are gone. Free the lookup table.
		free(mdp_ptr_RGB16toYUV);
		mdp_ptr_RGB16toYUV = NULL;
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
		case MDP_VAL_ROM_SIZE:
			// Read-only values.
			return -MDP_ERR_VAL_READ_ONLY;
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
			
			break;
		
		case MDP_VAL_ROM_SIZE:
			// ROM size.
			if (!Game)
				return 0;
			return Rom_Size;
		
		default:
			// Unknown value ID.
			return -MDP_ERR_UNKNOWN_VALID;
			break;
	}
	
	return MDP_ERR_OK;
}


/**
 * mdp_host_window_get_main(): Get a pointer to the main window.
 * @return Pointer to the main window.
 */
void* MDP_FNCALL mdp_host_window_get_main(void)
{
	#if defined(GENS_UI_GTK)
		return gens_window;
	#elif defined(GENS_UI_WIN32)
		return Gens_hWnd;
	#else
		fprintf(stderr, "%s(): Unknown UI. Returning NULL.\n", __func__);
		return NULL;
	#endif
}
