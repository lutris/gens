/***************************************************************************
 * Gens: MDP: Mega Drive Plugin - Error Codes.                             *
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

#ifndef __MDP_ERROR_H
#define __MDP_ERROR_H

// These error codes are typically returned as negative numbers.
// Example: return -MDP_ERR_FUNCTION_NOT_IMPLEMENTED.

// Low word: Error code.
// High word: Error category.

// General error codes.
#define MDP_ERR_GENERAL_CATEGORY		(0x0000 << 16)
#define	MDP_ERR_OK				(0x0000 | MDP_ERR_GENERAL_CATEGORY)
#define MDP_ERR_CANNOT_OPEN_DLL			(0x0001 | MDP_ERR_GENERAL_CATEGORY)
#define MDP_ERR_NO_MDP_SYMBOL			(0x0002 | MDP_ERR_GENERAL_CATEGORY)
#define MDP_ERR_INCORRECT_MAJOR_VERSION		(0x0003 | MDP_ERR_GENERAL_CATEGORY)
#define MDP_ERR_NEEDS_CPUFLAGS			(0x0004 | MDP_ERR_GENERAL_CATEGORY)
#define	MDP_ERR_FUNCTION_NOT_IMPLEMENTED	(0x0005 | MDP_ERR_GENERAL_CATEGORY)
#define MDP_ERR_UNKNOWN_PTRID			(0x0006 | MDP_ERR_GENERAL_CATEGORY)
#define	MDP_ERR_UNKNOWN_VALID			(0x0007 | MDP_ERR_GENERAL_CATEGORY)
#define	MDP_ERR_VAL_READ_ONLY			(0x0008 | MDP_ERR_GENERAL_CATEGORY)
#define MDP_ERR_UNSUPPORTED_UI			(0x0009 | MDP_ERR_GENERAL_CATEGORY)

// Renderer errors.
#define MDP_ERR_RENDER_CATEGORY			(0x0001 << 16)
#define MDP_ERR_RENDER_UNKNOWN			(0x0000 | MDP_ERR_RENDER_CATEGORY)
#define MDP_ERR_RENDER_INVALID_RENDERINFO	(0x0001 | MDP_ERR_RENDER_CATEGORY)

// Menu errors.
#define MDP_ERR_MENU_CATEGORY			(0x0002 << 16)
#define MDP_ERR_MENU_UNKNOWN			(0x0000 | MDP_ERR_MENU_CATEGORY)
#define MDP_ERR_MENU_INVALID_MENUID		(0x0001 | MDP_ERR_MENU_CATEGORY)
#define MDP_ERR_MENU_TOO_MANY_ITEMS		(0x0002 | MDP_ERR_MENU_CATEGORY)

// Event errors.
#define MDP_ERR_EVENT_CATEGORY			(0x0003 << 16)
#define MDP_ERR_EVENT_UNKNOWN			(0x0000 | MDP_ERR_EVENT_CATEGORY)
#define MDP_ERR_EVENT_INVALID_EVENTID		(0x0001 | MDP_ERR_EVENT_CATEGORY)
#define MDP_ERR_EVENT_NOT_REGISTERED		(0x0002 | MDP_ERR_EVENT_CATEGORY)
#define MDP_ERR_EVENT_ALREADY_REGISTERED	(0x0003 | MDP_ERR_EVENT_CATEGORY)

// Window errors.
#define MDP_ERR_WINDOW_CATEGORY			(0x0004 << 16)
#define MDP_ERR_WINDOW_UNKNOWN			(0x0000 | MDP_ERR_WINDOW_CATEGORY)
#define MDP_ERR_WINDOW_INVALID_WINDOW		(0x0001 | MDP_ERR_WINDOW_CATEGORY)
#define MDP_ERR_WINDOW_ALREADY_REGISTERED	(0x0002 | MDP_ERR_WINDOW_CATEGORY)

#endif /* __MDP_ERROR_H */
