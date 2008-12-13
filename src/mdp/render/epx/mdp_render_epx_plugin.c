/***************************************************************************
 * Gens: [MDP] EPX renderer. (Plugin Data File)                            *
 *                                                                         *
 * Original code from Gens Rerecording.                                    *
 * MDP version Copyright (c) 2008 by David Korth                           *
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

#include <stdint.h>
#include <string.h>

#include "mdp/mdp.h"
#include "mdp/mdp_cpuflags.h"

#include "mdp_render_epx.hpp"

static MDP_Desc_t MDP_Desc =
{
	.name = "EPX Renderer",
	.author_mdp = "David Korth",
	.author_orig = "Gens Rerecording developers",
	.description = "EPX renderer.",
	.website = "http://code.google.com/p/gens-rerecording/",
	.license = MDP_LICENSE_GPL_2,
	
	.icon = NULL,
	.iconLength = 0
};

MDP_Render_t mdp_render_t =
{
	.interfaceVersion = MDP_RENDER_INTERFACE_VERSION,
	.blit = mdp_render_epx_cpp,
	.scale = 2,
	.flags = 0,
	.tag = "EPX"
};

static MDP_Func_t MDP_Func =
{
	.init = mdp_render_epx_init,
	.end = mdp_render_epx_end
};

MDP_t mdp =
{
	// Plugin version information.
	.interfaceVersion = MDP_INTERFACE_VERSION,
	.pluginVersion = MDP_VERSION(1, 0, 0),
	
	// CPU flags.
	.cpuFlagsSupported = 0,
	.cpuFlagsRequired = 0,
	
	// UUID: b6e95b28-ee68-4954-a683-5c362bf9fa2f
	.uuid = {0xB6, 0xE9, 0x5B, 0x28,
		 0xEE, 0x68,
		 0x49, 0x54,
		 0xA6, 0x83,
		 0x5C, 0x36, 0x2B, 0xF9, 0xFA, 0x2F},
	
	// Description.
	.desc = &MDP_Desc,
	
	// Functions.
	.func = &MDP_Func
};
