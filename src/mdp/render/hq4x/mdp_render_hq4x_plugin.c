/***************************************************************************
 * MDP: hq4x renderer. (Plugin Data File)                                  *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2009 by David Korth                                  *
 * hq4x Copyright (c) 2003 by Maxim Stepin                                 *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU Lesser General Public License as published   *
 * by the Free Software Foundation; either version 2.1 of the License, or  *
 * (at your option) any later version.                                     *
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

#include "mdp_render_hq4x.h"

#include "mdp/mdp.h"
#include "mdp/mdp_cpuflags.h"

static mdp_desc_t mdp_desc =
{
	.name = "hq4x Renderer",
	.author_mdp = "David Korth",
	.author_orig = "Maxim Stepin",
	.description = "hq4x renderer.",
	.website = "http://www.hiend3d.com/",
	.license = MDP_LICENSE_LGPL_21
};

mdp_render_t mdp_render =
{
	.blit = mdp_render_hq4x_cpp,
	.tag = "hq4x",
	
	.scale = 4,
	.flags = MDP_RENDER_FLAG_RGB_555to555 |
		 MDP_RENDER_FLAG_RGB_565to565
};

static mdp_func_t mdp_func =
{
	.init = mdp_render_hq4x_init,
	.end = mdp_render_hq4x_end,
};

mdp_t mdp =
{
	// Plugin version information.
	.interfaceVersion = MDP_VERSION(1, 0, 0),
	.pluginVersion = MDP_VERSION(1, 0, 0),
	
	// CPU flags.
	.cpuFlagsSupported = MDP_CPUFLAG_X86_MMX,
	.cpuFlagsRequired = MDP_CPUFLAG_X86_MMX,
	
	// UUID: 5708aebf-d8ab-4e34-80cd-c66fd2b7f2c5
	.uuid = {0x57, 0x08, 0xAE, 0xBF,
		 0xD8, 0xAB,
		 0x4E, 0x34,
		 0x80, 0xCD,
		 0xC6, 0x6F, 0xD2, 0xB7, 0xF2, 0xC5},
	
	// Description.
	.desc = &mdp_desc,
	
	// Functions.
	.func = &mdp_func
};
