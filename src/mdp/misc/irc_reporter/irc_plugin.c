/***************************************************************************
 * MDP: IRC Reporter. (Plugin Data File)                                   *
 *                                                                         *
 * Copyright (c) 2009 by David Korth                                       *
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

#include "irc.hpp"

static mdp_desc_t mdp_desc =
{
	.name = "IRC Reporter",
	.author_mdp = "David Korth",
	.author_orig = NULL,
	.description = "Reports loaded games to IRC clients using the D-Bus protocol.",
	.website = NULL,
	.license = MDP_LICENSE_GPL_2,
	
	.icon = NULL,
	.iconLength = 0
};

static mdp_func_t mdp_func =
{
	.init = irc_init,
	.end = irc_end
};

mdp_t mdp =
{
	// Plugin version information.
	.interfaceVersion = MDP_INTERFACE_VERSION,
	.pluginVersion = MDP_VERSION(0, 0, 1),
	
	// CPU flags.
	.cpuFlagsSupported = 0,
	.cpuFlagsRequired = 0,
	
	// UUID: 5c5c31f4-4d5b-4dcd-ae0e-468dc9d650a9
	.uuid = {0x5C, 0x5C, 0x31, 0xF4,
		 0x4D, 0x5B,
		 0x4D, 0xCD,
		 0xAE, 0x0E,
		 0x46, 0x8D, 0xC9, 0xD6, 0x50, 0xA9},
	
	// Description.
	.desc = &mdp_desc,
	
	// Functions.
	.func = &mdp_func
};
