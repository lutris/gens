/***************************************************************************
 * Gens: [MDP] Sonic Gens. (ROM Type Definitions)                          *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * SGens Copyright (c) 2002 by LOst                                        *
 * MDP port Copyright (c) 2008-2009 by David Korth                         *
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

#include "sgens_rom_type.h"

#include <string.h>

const char* const sgens_ROM_type_name[SGENS_ROM_TYPE_MAX + 1] =
{
	"No ROM loaded.",
	"Unsupported Mega Drive ROM.",
	"Unsupported Mega CD game.",
	"Unsupported Sega 32X ROM.",
	"Unsupported Mega CD 32X game.",
	
	"Sonic 1 (Rev. 00)",
	"Sonic 1 (Rev. 01)",
	"Sonic 1 (Rev. XB)",
	
	"Sonic 2 (Prototype; Wai Version)",
	"Sonic 2 (Rev. 00)",
	"Sonic 2 (Rev. 01)",
	"Sonic 2 (Rev. 02)",
	
	"Sonic 3",
	"Sonic & Knuckles",
	"Knuckles in Sonic 2",
	NULL
};
