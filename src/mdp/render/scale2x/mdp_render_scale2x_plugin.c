/***************************************************************************
 * MDP: Scale2x renderer. (Plugin Data File)                               *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2009 by David Korth                                  *
 * Scale2x Copyright (c) 2001 by Andrea Mazzoleni                          *
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "mdp_render_scale2x.h"
#include "mdp_render_scale2x_icon.h"

#include "mdp/mdp.h"
#include "mdp/mdp_cpuflags.h"

static mdp_desc_t mdp_desc =
{
	.name = "Scale2x Renderer",
	.author_mdp = "David Korth",
	.author_orig = "Andrea Mazzoleni",
	.description = "Scale2x v2.2.",
	.website = "http://scale2x.sourceforge.net/",
	.license = MDP_LICENSE_GPL_2,
	
	.icon = mdp_render_scale2x_icon,
	.iconLength = sizeof(mdp_render_scale2x_icon)
};

mdp_render_t mdp_render =
{
	.blit = mdp_render_scale2x_cpp,
	.tag = "Scale2x",
	
	.scale = 2,
	.flags = MDP_RENDER_FLAG_RGB_555to555 |
		 MDP_RENDER_FLAG_RGB_565to565 |
		 MDP_RENDER_FLAG_RGB_888to888
};

static mdp_func_t mdp_func =
{
	.init = mdp_render_scale2x_init,
	.end = mdp_render_scale2x_end
};

mdp_t mdp =
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
	
	// UUID: 9f61e947-8a92-46c0-ad4c-f42e642d2257
	.uuid = {0x9F, 0x61, 0xE9, 0x47,
		 0x8A, 0x92,
		 0x46, 0xC0,
		 0xAD, 0x4C,
		 0xF4, 0x2E, 0x64, 0x2D, 0x22, 0x57},
	
	// Description.
	.desc = &mdp_desc,
	
	// Functions.
	.func = &mdp_func
};
