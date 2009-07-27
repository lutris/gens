/***************************************************************************
 * MDP: Interpolated 50% Scanline renderer. (Plugin Data File)             *
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

#include "mdp_render_interpolated_scanline_50.hpp"
#include "mdp_render_interpolated_scanline_50_icon.h"

#include "mdp/mdp.h"
#include "mdp/mdp_cpuflags.h"

static mdp_desc_t mdp_desc =
{
	.name = "Interpolated 50% Scanline Renderer",
	.author_mdp = "David Korth",
	.author_orig = "Stéphane Dallongeville",
	.description = "Interpolated 50% scanline renderer.",
	.website = NULL,
	.license = MDP_LICENSE_GPL_2,
	
	.icon = mdp_render_interpolated_scanline_50_icon,
	.iconLength = sizeof(mdp_render_interpolated_scanline_50_icon)
};

mdp_render_t mdp_render =
{
	.blit = mdp_render_interpolated_scanline_50_cpp,
	.tag = "Interpolated 50% Scanline",
	
	.scale = 2,
	.flags = MDP_RENDER_FLAG_RGB_555to555 |
		 MDP_RENDER_FLAG_RGB_565to565 |
		 MDP_RENDER_FLAG_RGB_888to888
};

static mdp_func_t mdp_func =
{
	.init = mdp_render_interpolated_scanline_50_init,
	.end = mdp_render_interpolated_scanline_50_end
};

mdp_t mdp =
{
	// Plugin version information.
	.interfaceVersion = MDP_VERSION(1, 0, 0),
	.pluginVersion = MDP_VERSION(1, 0, 0),
	
	// CPU flags.
	.cpuFlagsSupported = 0,
	.cpuFlagsRequired = 0,
	
	// UUID: 3c80e487-8a32-48db-9d55-9354c1ea7c28
	.uuid = {0x3C, 0x80, 0xE4, 0x87,
		 0x8A, 0x32,
		 0x48, 0xDB,
		 0x9D, 0x55,
		 0x93, 0x54, 0xC1, 0xEA, 0x7C, 0x28},
	
	// Description.
	.desc = &mdp_desc,
	
	// Functions.
	.func = &mdp_func
};
