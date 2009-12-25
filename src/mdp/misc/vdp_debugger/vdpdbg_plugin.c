/***************************************************************************
 * MDP: VDP Debugger. (Plugin Data File)                                   *
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

#include "mdp/mdp.h"
#include "mdp/mdp_cpuflags.h"

#include "vdpdbg.h"

static mdp_desc_t mdp_desc =
{
	.name = "VDP Debugger",
	.author_mdp = "David Korth",
	.author_orig = "Stéphane Dallongeville",
	.description = "MDP version of the internal Gens/GS VDP debugger with extra features.",
	.website = NULL,
	.license = MDP_LICENSE_GPL_2,
	
	.icon = NULL,
	.iconLength = 0
};

static mdp_func_t mdp_func =
{
	.init = vdpdbg_init,
	.end = vdpdbg_end
};

mdp_t mdp =
{
	// Plugin version information.
	.interfaceVersion = MDP_INTERFACE_VERSION,
	.pluginVersion = MDP_VERSION(0, 0, 1),
	
	// CPU flags.
	.cpuFlagsSupported = 0,
	.cpuFlagsRequired = 0,
	
	// UUID: ac21bc5b-ad11-43fe-a163-5749f86feb8c
	.uuid = {0xAC, 0x21, 0xBC, 0x5B,
		 0xAD, 0x11,
		 0x43, 0xFE,
		 0xA1, 0x63,
		 0x57, 0x49, 0xF8, 0x6F, 0xEB, 0x8C},
	
	// Description.
	.desc = &mdp_desc,
	
	// Functions.
	.func = &mdp_func
};
