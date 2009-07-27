/***************************************************************************
 * Gens: [MDP] 1x renderer. (Plugin Data File)                             *
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

#include "mdp_render_1x.hpp"
#include "mdp_render_1x_icon.h"

static mdp_desc_t mdp_desc =
{
	.name = "Normal Renderer",
	.author_mdp = "David Korth",
	.author_orig = "Stéphane Dallongeville",
	.description = "Normal 1x renderer.",
	.website = NULL,
	.license = MDP_LICENSE_GPL_2,
	
	.icon = mdp_render_1x_icon,
	.iconLength = sizeof(mdp_render_1x_icon)
};

mdp_render_t mdp_render_1x_render_t =
{
	.blit = mdp_render_1x_cpp,
	.tag = "Normal",
	
	.scale = 1,
	.flags = MDP_RENDER_FLAG_RGB_555to555 |
		 MDP_RENDER_FLAG_RGB_565to565 |
		 MDP_RENDER_FLAG_RGB_888to888
};

static mdp_func_t mdp_func =
{
	.init = mdp_render_1x_init,
	.end = mdp_render_1x_end
};

mdp_t mdp_render_1x =
{
	// Plugin version information.
	.interfaceVersion = MDP_VERSION(1, 0, 0),
	.pluginVersion = MDP_VERSION(1, 0, 0),
	
	// CPU flags.
#ifdef GENS_X86_ASM
	.cpuFlagsSupported = MDP_CPUFLAG_X86_MMX,
	.cpuFlagsRequired = 0,
#else
	.cpuFlagsSupported = 0,
	.cpuFlagsRequired = 0,
#endif
	
	// UUID: b3366576-f17a-4f5a-a923-a654c513b185
	.uuid = {0xB3, 0x36, 0x65, 0x76,
		 0xF1, 0x7A,
		 0x4F, 0x5A,
		 0xA9, 0x23,
		 0xA6, 0x54, 0xC5, 0x13, 0xB1, 0x85},
	
	// Description.
	.desc = &mdp_desc,
	
	// Functions.
	.func = &mdp_func
};
