/***************************************************************************
 * Gens: Plugin Manager Window. (Common Functions)                         *
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

#ifndef GENS_PLUGIN_MANAGER_WINDOW_COMMON_HPP
#define GENS_PLUGIN_MANAGER_WINDOW_COMMON_HPP

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef GENS_PNG

#include <png.h>
extern const unsigned char	*pmgr_window_png_dataptr;
extern unsigned int		pmgr_window_png_datalen;
extern unsigned int		pmgr_window_png_datapos;

void pmgr_window_png_user_read_data(png_structp png_ptr, png_bytep data, png_size_t length);
#endif

#ifdef __cplusplus

#include <string>
#include <stdint.h>

std::string GetCPUFlags_string(const uint32_t cpuFlagsRequired,
			       const uint32_t cpuFlagsSupported,
			       const bool formatting);

std::string UUIDtoString(const unsigned char *uuid);

#endif /* __cplusplus */

#endif /* GENS_PLUGIN_MANAGER_WINDOW_COMMON_HPP */
