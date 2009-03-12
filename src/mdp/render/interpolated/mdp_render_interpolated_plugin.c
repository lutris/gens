/***************************************************************************
 * Gens: [MDP] Interpolated renderer. (Plugin Data File)                   *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
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

#include "mdp_render_interpolated.hpp"
#include "mdp_render_interpolated_icon.h"

static mdp_desc_t mdp_desc =
{
	.name = "Interpolated Renderer",
	.author_mdp = "David Korth",
	.author_orig = "Stéphane Dallongeville",
	.description = "Interpolated renderer.",
	.website = NULL,
	.license = MDP_LICENSE_GPL_2,
	
	.icon = mdp_render_interpolated_icon,
	.iconLength = sizeof(mdp_render_interpolated_icon)
};

mdp_render_t mdp_render =
{
	.interfaceVersion = MDP_RENDER_INTERFACE_VERSION,
	.scale = 2,
	
	.blit = mdp_render_interpolated_cpp,
	.tag = "Interpolated",
	
	.flags = MDP_RENDER_FLAG_RGB555 |
		 MDP_RENDER_FLAG_RGB565 |
		 MDP_RENDER_FLAG_RGB888
};

static mdp_func_t mdp_func =
{
	.init = mdp_render_interpolated_init,
	.end = mdp_render_interpolated_end
};

mdp_t mdp =
{
	// Plugin version information.
	.interfaceVersion = MDP_INTERFACE_VERSION,
	.pluginVersion = MDP_VERSION(1, 0, 0),
	
	// CPU flags.
	.cpuFlagsSupported = 0,
	.cpuFlagsRequired = 0,
	
	// UUID: fd39b461-9488-4eff-b401-a9e764fdc0b7
	.uuid = {0xFD, 0x39, 0xB4, 0x61,
		 0x94, 0x88,
		 0x4E, 0xFF,
		 0xB4, 0x01,
		 0xA9, 0xE7, 0x64, 0xFD, 0xC0, 0xB7},
	
	// Description.
	.desc = &mdp_desc,
	
	// Functions.
	.func = &mdp_func
};
