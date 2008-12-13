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
#define MDP_ERRCAT_GENERAL			(0x0000 << 4)
#define	MDP_ERR_OK				(0x0000 | MDP_ERRCAT_GENERAL)
#define	MDP_ERR_FUNCTION_NOT_IMPLEMENTED	(0x0001 | MDP_ERRCAT_GENERAL)
#define MDP_ERR_UNKNOWN_PTRID			(0x0002 | MDP_ERRCAT_GENERAL)

// Renderer errors.
#define MDP_ERRCAT_RENDER			(0x0001 << 4)
#define MDP_ERR_RENDER_UNKNOWN			(0x0000 | MDP_ERRCAT_RENDER)
#define MDP_ERR_RENDER_INVALID_RENDERINFO	(0x0002 | MDP_ERRCAT_RENDER)

#endif /* __MDP_ERROR_H */
