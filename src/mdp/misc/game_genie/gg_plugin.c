/***************************************************************************
 * MDP: Game Genie. (Plugin Data File)                                     *
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

#include "gg.hpp"
#include "gg_icon.h"

static mdp_desc_t mdp_desc =
{
	.name = "Game Genie",
	.author_mdp = "David Korth",
	.author_orig = "Stéphane Dallongeville",
	.description = "Game Genie and patch code plugin.",
	.website = NULL,
	.license = MDP_LICENSE_GPL_2,
	
	.icon = gg_icon,
	.iconLength = sizeof(gg_icon)
};

static mdp_func_t mdp_func =
{
	.init = gg_init,
	.end = gg_end
};

mdp_t mdp =
{
	// Plugin version information.
	.interfaceVersion = MDP_INTERFACE_VERSION,
	.pluginVersion = MDP_VERSION(1, 0, 0),
	
	// CPU flags.
	.cpuFlagsSupported = 0,
	.cpuFlagsRequired = 0,
	
	// UUID: 0e34744a-54bc-41b9-b91b-40ce3fc750bc
	.uuid = {0x0E, 0x34, 0x74, 0x4A,
		 0x54, 0xBC,
		 0x41, 0xB9,
		 0xB9, 0x1B,
		 0x40, 0xCE, 0x3F, 0xC7, 0x50, 0xBC},
	
	// Description.
	.desc = &mdp_desc,
	
	// Functions.
	.func = &mdp_func
};
