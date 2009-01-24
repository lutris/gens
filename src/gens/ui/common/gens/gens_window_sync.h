/***************************************************************************
 * Gens: Main Window - Synchronization Functions.                          *
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

#ifndef GENS_COMMON_GENS_WINDOW_SYNC_H
#define GENS_COMMON_GENS_WINDOW_SYNC_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

extern const char* const gws_rom_format_prefix[6];

// Macro to convert width*height into a 32-bit word.
#define GENS_GWS_RES(width, height)	\
	(((width & 0xFFFF) << 16) | (height & 0xFFFF))

#ifdef GENS_OPENGL
extern const uint32_t gws_opengl_resolutions[][2];
#endif /* GENS_OPENGL */

#ifdef GENS_DEBUGGER
extern const char* const gws_debug_items[];
#endif /* GENS_DEBUGGER */

#ifdef __cplusplus
}
#endif

#endif /* GENS_COMMON_GENS_WINDOW_SYNC_H */
