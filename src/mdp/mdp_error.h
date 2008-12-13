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

// No error.
#define	MDP_ERR_OK				0

// Function not implemented.
#define	MDP_ERR_FUNCTION_NOT_IMPLEMENTED	1

// Unknown pointer ID.
#define MDP_ERR_UNKNOWN_PTRID			2

#endif /* __MDP_ERROR_H */
