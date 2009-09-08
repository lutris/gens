/***************************************************************************
 * Gens: Command line parser.                                              *
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

#ifndef GENS_PARSE_HPP
#define GENS_PARSE_HPP

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// TODO: Move GENS_PATH_MAX out of g_main.hpp.
#include "g_main.hpp"

#ifdef __cplusplus
extern "C" {
#endif

// Startup mode.
typedef enum _Gens_StartupMode_t
{
	GSM_IDLE = 0,
	GSM_ROM = 1,
#ifdef GENS_CDROM
	GSM_BOOT_CD = 2,
#endif
	GSM_MAX
} Gens_StartupMode_t;

// Startup information.
typedef struct _Gens_StartupInfo_t
{
	Gens_StartupMode_t mode;
	char filename[GENS_PATH_MAX];
#if defined(GENS_OS_WIN32) && !defined(GENS_WIN32_CONSOLE)
	int enable_debug_console;
#endif
} Gens_StartupInfo_t;

Gens_StartupInfo_t* parse_args(int argc, char *argv[]);

#ifdef __cplusplus
}
#endif

#endif /* GENS_PARSE_HPP */
