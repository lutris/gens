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

/**
 * These error codes are typically returned as negative numbers.
 * Example: return -MDP_ERR_FUNCTION_NOT_IMPLEMENTED.
 */

/**
 * Low word: Error code.
 * High word: Error category.
 */

/** General errors. **/
/*! BEGIN: MDP v1.0 general errors. !*/
#define MDP_ERR_GENERAL_CATEGORY		((int)(0x0000 << 16))
#define	MDP_ERR_OK				((int)(0x0000 | MDP_ERR_GENERAL_CATEGORY))
#define MDP_ERR_CANNOT_OPEN_DLL			((int)(0x0001 | MDP_ERR_GENERAL_CATEGORY))
#define MDP_ERR_NO_MDP_SYMBOL			((int)(0x0002 | MDP_ERR_GENERAL_CATEGORY))
#define MDP_ERR_INCORRECT_MAJOR_VERSION		((int)(0x0003 | MDP_ERR_GENERAL_CATEGORY))
#define MDP_ERR_DUPLICATE_UUID			((int)(0x0004 | MDP_ERR_GENERAL_CATEGORY))
#define MDP_ERR_NEEDS_CPUFLAGS			((int)(0x0005 | MDP_ERR_GENERAL_CATEGORY))
#define MDP_ERR_INVALID_LICENSE			((int)(0x0006 | MDP_ERR_GENERAL_CATEGORY))

#define	MDP_ERR_FUNCTION_NOT_IMPLEMENTED	((int)(0x0101 | MDP_ERR_GENERAL_CATEGORY))
#define MDP_ERR_INVALID_PARAMETERS		((int)(0x0102 | MDP_ERR_GENERAL_CATEGORY))
#define MDP_ERR_OUT_OF_MEMORY			((int)(0x0103 | MDP_ERR_GENERAL_CATEGORY))

#define MDP_ERR_UNKNOWN_PTRID			((int)(0x0201 | MDP_ERR_GENERAL_CATEGORY))
#define	MDP_ERR_UNKNOWN_VALID			((int)(0x0202 | MDP_ERR_GENERAL_CATEGORY))
#define	MDP_ERR_VAL_READ_ONLY			((int)(0x0203 | MDP_ERR_GENERAL_CATEGORY))
#define MDP_ERR_UNSUPPORTED_UI			((int)(0x0204 | MDP_ERR_GENERAL_CATEGORY))
#define MDP_ERR_ROM_NOT_LOADED			((int)(0x0205 | MDP_ERR_GENERAL_CATEGORY))
#define MDP_ERR_UNKNOWN				((int)(0xFFFF | MDP_ERR_GENERAL_CATEGORY))
/*! END: MDP v1.0 general errors. !*/

/** Renderer errors. **/
/*! BEGIN: MDP v1.0 renderer errors. !*/
#define MDP_ERR_RENDER_CATEGORY			((int)(0x0001 << 16))
#define MDP_ERR_RENDER_UNKNOWN			((int)(0x0000 | MDP_ERR_RENDER_CATEGORY))
#define MDP_ERR_RENDER_DUPLICATE_TAG		((int)(0x0001 | MDP_ERR_RENDER_CATEGORY))
#define MDP_ERR_RENDER_INVALID_RENDERINFO	((int)(0x0002 | MDP_ERR_RENDER_CATEGORY))
#define MDP_ERR_RENDER_UNSUPPORTED_FLAG		((int)(0x0003 | MDP_ERR_RENDER_CATEGORY))
#define MDP_ERR_RENDER_UNSUPPORTED_VMODE	((int)(0x0004 | MDP_ERR_RENDER_CATEGORY))
/*! END: MDP v1.0 renderer errors. !*/

/** Memory errors. **/
/*! BEGIN: MDP v1.0 memory errors. !*/
#define MDP_ERR_MEM_CATEGORY			((int)(0x0002 << 16))
#define MDP_ERR_MEM_UNKNOWN			((int)(0x0000 | MDP_ERR_MEM_CATEGORY))
#define MDP_ERR_MEM_INVALID_MEMID		((int)(0x0001 | MDP_ERR_MEM_CATEGORY))
#define MDP_ERR_MEM_NOT_ALLOCATED		((int)(0x0002 | MDP_ERR_MEM_CATEGORY))
#define MDP_ERR_MEM_OUT_OF_RANGE		((int)(0x0003 | MDP_ERR_MEM_CATEGORY))
#define MDP_ERR_MEM_NOT_RESIZABLE		((int)(0x0004 | MDP_ERR_MEM_CATEGORY))
#define MDP_ERR_MEM_UNALIGNED			((int)(0x0005 | MDP_ERR_MEM_CATEGORY))
/*! END: MDP v1.0 memory errors. !*/

/** Register errors. **/
/*! BEGIN: MDP v1.0 register errors. !*/
#define MDP_ERR_REG_CATEGORY			((int)(0x0003 << 16))
#define MDP_ERR_REG_UNKNOWN			((int)(0x0000 | MDP_ERR_MEM_CATEGORY))
#define MDP_ERR_REG_INVALID_ICID		((int)(0x0001 | MDP_ERR_MEM_CATEGORY))
#define MDP_ERR_REG_INVALID_REGID		((int)(0x0002 | MDP_ERR_MEM_CATEGORY))
#define MDP_ERR_REG_NOT_ALLOCATED		((int)(0x0003 | MDP_ERR_MEM_CATEGORY))
#define MDP_ERR_REG_READ_ONLY			((int)(0x0004 | MDP_ERR_MEM_CATEGORY))
#define MDP_ERR_REG_WRITE_ONLY			((int)(0x0005 | MDP_ERR_MEM_CATEGORY))
/*! END: MDP v1.0 register errors. !*/

/** Menu errors. **/
/*! BEGIN: MDP v1.0 menu errors. !*/
#define MDP_ERR_MENU_CATEGORY			((int)(0x0004 << 16))
#define MDP_ERR_MENU_UNKNOWN			((int)(0x0000 | MDP_ERR_MENU_CATEGORY))
#define MDP_ERR_MENU_INVALID_MENUID		((int)(0x0001 | MDP_ERR_MENU_CATEGORY))
#define MDP_ERR_MENU_TOO_MANY_ITEMS		((int)(0x0002 | MDP_ERR_MENU_CATEGORY))
/*! END: MDP v1.0 menu errors. !*/

/** Event errors. **/
/*! BEGIN: MDP v1.0 event errors. !*/
#define MDP_ERR_EVENT_CATEGORY			((int)(0x0005 << 16))
#define MDP_ERR_EVENT_UNKNOWN			((int)(0x0000 | MDP_ERR_EVENT_CATEGORY))
#define MDP_ERR_EVENT_INVALID_EVENTID		((int)(0x0001 | MDP_ERR_EVENT_CATEGORY))
#define MDP_ERR_EVENT_NOT_REGISTERED		((int)(0x0002 | MDP_ERR_EVENT_CATEGORY))
#define MDP_ERR_EVENT_ALREADY_REGISTERED	((int)(0x0003 | MDP_ERR_EVENT_CATEGORY))
/*! END: MDP v1.0 event errors. !*/

/** Window errors. **/
/*! BEGIN: MDP v1.0 window errors. !*/
#define MDP_ERR_WINDOW_CATEGORY			((int)(0x0006 << 16))
#define MDP_ERR_WINDOW_UNKNOWN			((int)(0x0000 | MDP_ERR_WINDOW_CATEGORY))
#define MDP_ERR_WINDOW_INVALID_WINDOW		((int)(0x0001 | MDP_ERR_WINDOW_CATEGORY))
#define MDP_ERR_WINDOW_ALREADY_REGISTERED	((int)(0x0002 | MDP_ERR_WINDOW_CATEGORY))
/*! END: MDP v1.0 window errors. !*/

/** Emulator Control errors. **/
/*! BEGIN: MDP v1.0 emulator control errors. !*/
#define MDP_ERR_EMUCTRL_CATEGORY			((int)(0x0007 << 16))
#define MDP_ERR_EMUCTRL_UNKNOWN				((int)(0x0000 | MDP_ERR_DIR_CATEGORY))
#define MDP_ERR_EMUCTRL_INVALID_FUNCTION		((int)(0x0001 | MDP_ERR_DIR_CATEGORY))
#define MDP_ERR_EMUCTRL_FUNCTION_NOT_IMPLEMENTED	((int)(0x0002 | MDP_ERR_DIR_CATEGORY))
/*! END: MDP v1.0 emulator control errors. !*/

/** Directory errors. **/
/*! BEGIN: MDP v1.0 directory errors. !*/
#define MDP_ERR_DIR_CATEGORY			((int)(0x0008 << 16))
#define MDP_ERR_DIR_UNKNOWN			((int)(0x0000 | MDP_ERR_DIR_CATEGORY))
#define MDP_ERR_DIR_INVALID_DIRID		((int)(0x0001 | MDP_ERR_DIR_CATEGORY))
/*! END: MDP v1.0 directory errors. !*/

/** Compression errors. **/
/*! BEGIN: MDP v1.0 compression errors. !*/
#define MDP_ERR_Z_CATEGORY			((int)(0x0009 << 16))
#define MDP_ERR_Z_UNKNOWN			((int)(0x0000 | MDP_ERR_Z_CATEGORY))
#define MDP_ERR_Z_ARCHIVE_NOT_FOUND		((int)(0x0001 | MDP_ERR_Z_CATEGORY))
#define MDP_ERR_Z_EXE_NOT_FOUND			((int)(0x0002 | MDP_ERR_Z_CATEGORY))
#define MDP_ERR_Z_ARCHIVE_NOT_SUPPORTED		((int)(0x0003 | MDP_ERR_Z_CATEGORY))
#define MDP_ERR_Z_CANT_OPEN_ARCHIVE		((int)(0x0004 | MDP_ERR_Z_CATEGORY))
#define MDP_ERR_Z_NO_FILES_IN_ARCHIVE		((int)(0x0005 | MDP_ERR_Z_CATEGORY))
#define MDP_ERR_Z_FILE_NOT_FOUND_IN_ARCHIVE	((int)(0x0006 | MDP_ERR_Z_CATEGORY))
/*! END: MDP v1.0 compression errors. !*/

#endif /* __MDP_ERROR_H */
