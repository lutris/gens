/***************************************************************************
 * MDP: Sonic Gens. (Plugin Data File)                                     *
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

#include "mdp/mdp.h"
#include "mdp/mdp_cpuflags.h"

#include "sgens.h"
#include "sgens_icon.h"

static mdp_desc_t mdp_desc =
{
	.name = "Sonic Gens",
	.author_mdp = "David Korth",
	.author_orig = "LOst",
	.description = "Allows you to view various information in memory in several Sonic games.",
	.website = NULL,
	.license = MDP_LICENSE_GPL_2,
	
	.icon = sgens_icon,
	.iconLength = sizeof(sgens_icon)
};

static mdp_func_t mdp_func =
{
	.init = sgens_init,
	.end = sgens_end
};

mdp_t mdp =
{
	// Plugin version information.
	.interfaceVersion = MDP_INTERFACE_VERSION,
	.pluginVersion = MDP_VERSION(1, 0, 0),
	
	// CPU flags.
	.cpuFlagsSupported = 0,
	.cpuFlagsRequired = 0,
	
	// UUID: 64b01d98-9549-4b08-abe0-93012a015683
	.uuid = {0x64, 0xB0, 0x1D, 0x98,
		 0x95, 0x49,
		 0x4B, 0x08,
		 0xAB, 0xE0,
		 0x93, 0x01, 0x2A, 0x01, 0x56, 0x83},
	
	// Description.
	.desc = &mdp_desc,
	
	// Functions.
	.func = &mdp_func
};
