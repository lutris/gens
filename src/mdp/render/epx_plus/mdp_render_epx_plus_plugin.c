/***************************************************************************
 * MDP: EPX Plus renderer. (Plugin Data File)                              *
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

#include "mdp_render_epx_plus.hpp"

#include "mdp/mdp.h"
#include "mdp/mdp_cpuflags.h"

static mdp_desc_t mdp_desc =
{
	.name = "EPX Plus Renderer",
	.author_mdp = "David Korth",
	.author_orig = "Gens Rerecording developers",
	.description = "EPX Plus renderer.",
	.website = "http://code.google.com/p/gens-rerecording/",
	.license = MDP_LICENSE_GPL_2,
	
	.icon = NULL,
	.iconLength = 0
};

mdp_render_t mdp_render =
{
	.blit = mdp_render_epx_plus_cpp,
	.tag = "EPX Plus",
	
	.scale = 2,
	.flags = MDP_RENDER_FLAG_RGB_555to555 |
		 MDP_RENDER_FLAG_RGB_565to565 |
		 MDP_RENDER_FLAG_RGB_888to888
};

static mdp_func_t mdp_func =
{
	.init = mdp_render_epx_plus_init,
	.end = mdp_render_epx_plus_end
};

mdp_t mdp =
{
	// Plugin version information.
	.interfaceVersion = MDP_VERSION(1, 0, 0),
	.pluginVersion = MDP_VERSION(1, 0, 0),
	
	// CPU flags.
	.cpuFlagsSupported = 0,
	.cpuFlagsRequired = 0,
	
	// UUID: 7473b2a7-ef03-48a4-a791-158b9006ef88
	.uuid = {0x74, 0x73, 0xB2, 0xA7,
		 0xEF, 0x03,
		 0x48, 0xA4,
		 0xA7, 0x91,
		 0x15, 0x8B, 0x90, 0x06, 0xEF, 0x88},
	
	// Description.
	.desc = &mdp_desc,
	
	// Functions.
	.func = &mdp_func
};
