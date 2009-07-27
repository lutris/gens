/***************************************************************************
 * MDP: EPX renderer. (Plugin Data File)                                   *
 *                                                                         *
 * Original code from Gens Rerecording.                                    *
 * MDP version Copyright (c) 2008-2009 by David Korth                      *
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

#include "mdp_render_epx.hpp"

#include "mdp/mdp.h"
#include "mdp/mdp_cpuflags.h"

static mdp_desc_t mdp_desc =
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

mdp_render_t mdp_render =
{
	.blit = mdp_render_epx_cpp,
	.tag = "EPX",
	
	.scale = 2,
	.flags = MDP_RENDER_FLAG_RGB_555to555 |
		 MDP_RENDER_FLAG_RGB_565to565 |
		 MDP_RENDER_FLAG_RGB_888to888
};

static mdp_func_t mdp_func =
{
	.init = mdp_render_epx_init,
	.end = mdp_render_epx_end
};

mdp_t mdp =
{
	// Plugin version information.
	.interfaceVersion = MDP_VERSION(1, 0, 0),
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
	.desc = &mdp_desc,
	
	// Functions.
	.func = &mdp_func
};
