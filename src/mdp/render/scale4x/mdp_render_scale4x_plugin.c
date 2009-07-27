/***************************************************************************
 * MDP: Scale4x renderer. (Plugin Data File)                               *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2009 by David Korth                                  *
 * Scale4x Copyright (c) 2001 by Andrea Mazzoleni                          *
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

#include "mdp_render_scale4x.h"
#include "mdp_render_scale4x_icon.h"

#include "mdp/mdp.h"
#include "mdp/mdp_cpuflags.h"

static mdp_desc_t mdp_desc =
{
	.name = "Scale4x Renderer",
	.author_mdp = "David Korth",
	.author_orig = "Andrea Mazzoleni",
	.description = "Scale4x v2.2.",
	.website = "http://scale2x.sourceforge.net/",
	.license = MDP_LICENSE_GPL_2,
	
	.icon = mdp_render_scale4x_icon,
	.iconLength = sizeof(mdp_render_scale4x_icon)
};

mdp_render_t mdp_render =
{
	.blit = mdp_render_scale4x_cpp,
	.tag = "Scale4x",
	
	.scale = 4,
	.flags = MDP_RENDER_FLAG_RGB_555to555 |
		 MDP_RENDER_FLAG_RGB_565to565 |
		 MDP_RENDER_FLAG_RGB_888to888
};

static mdp_func_t mdp_func =
{
	.init = mdp_render_scale4x_init,
	.end = mdp_render_scale4x_end
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
	
	// UUID: 46e175f5-0949-40c2-b414-6635e0e550ee
	.uuid = {0x46, 0xE1, 0x75, 0xF5,
		 0x09, 0x49,
		 0x40, 0xC2,
		 0xB4, 0x14,
		 0x66, 0x35, 0xE0, 0xE5, 0x50, 0xEE},
	
	// Description.
	.desc = &mdp_desc,
	
	// Functions.
	.func = &mdp_func
};
