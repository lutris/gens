/***************************************************************************
 * MDP: Mega Drive Plugins - Error Codes.                                  *
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

#ifndef __MDP_ERROR_H
#define __MDP_ERROR_H

#include <stdint.h>

/**
 * These error codes are typically returned as negative numbers.
 * Example: return -MDP_ERR_FUNCTION_NOT_IMPLEMENTED.
 */

/**
 * Low word: Error code.
 * High word: Error category.
 */

/* General error codes. */
#define MDP_ERR_GENERAL_CATEGORY		((uint32_t)(0x0000 << 16))
#define	MDP_ERR_OK				((uint32_t)(0x0000 | MDP_ERR_GENERAL_CATEGORY))
#define MDP_ERR_CANNOT_OPEN_DLL			((uint32_t)(0x0001 | MDP_ERR_GENERAL_CATEGORY))
#define MDP_ERR_NO_MDP_SYMBOL			((uint32_t)(0x0002 | MDP_ERR_GENERAL_CATEGORY))
#define MDP_ERR_INCORRECT_MAJOR_VERSION		((uint32_t)(0x0003 | MDP_ERR_GENERAL_CATEGORY))
#define MDP_ERR_NEEDS_CPUFLAGS			((uint32_t)(0x0004 | MDP_ERR_GENERAL_CATEGORY))
#define MDP_ERR_INVALID_LICENSE			((uint32_t)(0x0005 | MDP_ERR_GENERAL_CATEGORY))

#define	MDP_ERR_FUNCTION_NOT_IMPLEMENTED	((uint32_t)(0x0101 | MDP_ERR_GENERAL_CATEGORY))
#define MDP_ERR_INVALID_PARAMETERS		((uint32_t)(0x0102 | MDP_ERR_GENERAL_CATEGORY))
#define MDP_ERR_OUT_OF_MEMORY			((uint32_t)(0x0103 | MDP_ERR_GENERAL_CATEGORY))

#define MDP_ERR_UNKNOWN_PTRID			((uint32_t)(0x0201 | MDP_ERR_GENERAL_CATEGORY))
#define	MDP_ERR_UNKNOWN_VALID			((uint32_t)(0x0202 | MDP_ERR_GENERAL_CATEGORY))
#define	MDP_ERR_VAL_READ_ONLY			((uint32_t)(0x0203 | MDP_ERR_GENERAL_CATEGORY))
#define MDP_ERR_UNSUPPORTED_UI			((uint32_t)(0x0204 | MDP_ERR_GENERAL_CATEGORY))
#define MDP_ERR_ROM_NOT_LOADED			((uint32_t)(0x0205 | MDP_ERR_GENERAL_CATEGORY))
#define MDP_ERR_UNKNOWN				((uint32_t)(0xFFFF | MDP_ERR_GENERAL_CATEGORY))

/* Renderer errors. */
#define MDP_ERR_RENDER_CATEGORY			((uint32_t)(0x0001 << 16))
#define MDP_ERR_RENDER_UNKNOWN			((uint32_t)(0x0000 | MDP_ERR_RENDER_CATEGORY))
#define MDP_ERR_RENDER_INVALID_RENDERINFO	((uint32_t)(0x0001 | MDP_ERR_RENDER_CATEGORY))

/* Memory errors. */
#define MDP_ERR_MEM_CATEGORY			((uint32_t)(0x0002 << 16))
#define MDP_ERR_MEM_UNKNOWN			((uint32_t)(0x0000 | MDP_ERR_MEM_CATEGORY))
#define MDP_ERR_MEM_INVALID_MEMID		((uint32_t)(0x0001 | MDP_ERR_MEM_CATEGORY))
#define MDP_ERR_MEM_NOT_ALLOCATED		((uint32_t)(0x0002 | MDP_ERR_MEM_CATEGORY))
#define MDP_ERR_MEM_OUT_OF_RANGE		((uint32_t)(0x0003 | MDP_ERR_MEM_CATEGORY))
#define MDP_ERR_MEM_NOT_RESIZABLE		((uint32_t)(0x0004 | MDP_ERR_MEM_CATEGORY))
#define MDP_ERR_MEM_UNALIGNED			((uint32_t)(0x0005 | MDP_ERR_MEM_CATEGORY))

/* Register errors. */
#define MDP_ERR_REG_CATEGORY			((uint32_t)(0x0003 << 16))
#define MDP_ERR_REG_UNKNOWN			((uint32_t)(0x0000 | MDP_ERR_MEM_CATEGORY))
#define MDP_ERR_REG_INVALID_ICID		((uint32_t)(0x0001 | MDP_ERR_MEM_CATEGORY))
#define MDP_ERR_REG_INVALID_REGID		((uint32_t)(0x0002 | MDP_ERR_MEM_CATEGORY))
#define MDP_ERR_REG_NOT_ALLOCATED		((uint32_t)(0x0003 | MDP_ERR_MEM_CATEGORY))

/* Menu errors. */
#define MDP_ERR_MENU_CATEGORY			((uint32_t)(0x0003 << 16))
#define MDP_ERR_MENU_UNKNOWN			((uint32_t)(0x0000 | MDP_ERR_MENU_CATEGORY))
#define MDP_ERR_MENU_INVALID_MENUID		((uint32_t)(0x0001 | MDP_ERR_MENU_CATEGORY))
#define MDP_ERR_MENU_TOO_MANY_ITEMS		((uint32_t)(0x0002 | MDP_ERR_MENU_CATEGORY))

/* Event errors. */
#define MDP_ERR_EVENT_CATEGORY			((uint32_t)(0x0004 << 16))
#define MDP_ERR_EVENT_UNKNOWN			((uint32_t)(0x0000 | MDP_ERR_EVENT_CATEGORY))
#define MDP_ERR_EVENT_INVALID_EVENTID		((uint32_t)(0x0001 | MDP_ERR_EVENT_CATEGORY))
#define MDP_ERR_EVENT_NOT_REGISTERED		((uint32_t)(0x0002 | MDP_ERR_EVENT_CATEGORY))
#define MDP_ERR_EVENT_ALREADY_REGISTERED	((uint32_t)(0x0003 | MDP_ERR_EVENT_CATEGORY))

/* Window errors. */
#define MDP_ERR_WINDOW_CATEGORY			((uint32_t)(0x0005 << 16))
#define MDP_ERR_WINDOW_UNKNOWN			((uint32_t)(0x0000 | MDP_ERR_WINDOW_CATEGORY))
#define MDP_ERR_WINDOW_INVALID_WINDOW		((uint32_t)(0x0001 | MDP_ERR_WINDOW_CATEGORY))
#define MDP_ERR_WINDOW_ALREADY_REGISTERED	((uint32_t)(0x0002 | MDP_ERR_WINDOW_CATEGORY))

/* Directory errors. */
#define MDP_ERR_DIR_CATEGORY			((uint32_t)(0x0006 << 16))
#define MDP_ERR_DIR_UNKNOWN			((uint32_t)(0x0000 | MDP_ERR_DIR_CATEGORY))
#define MDP_ERR_DIR_INVALID_DIRID		((uint32_t)(0x0001 | MDP_ERR_DIR_CATEGORY))

#endif /* __MDP_ERROR_H */
