/***************************************************************************
 * MDP: 2xSaI renderer. (Plugin Data File)                                 *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2009 by David Korth                                  *
 * 2xSaI Copyright (c) by Derek Liauw Kie Fa and Robert J. Ohannessian     *
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

#include "mdp_render_2xsai.h"

#include "mdp/mdp.h"
#include "mdp/mdp_cpuflags.h"

static mdp_desc_t mdp_desc =
{
	.name = "2xSaI Renderer",
	.author_mdp = "David Korth",
	.author_orig = "Derek Liauw Kie Fa and Robert J. Ohannessian",
	.description = "2x Scale and Interpolation renderer.",
	.website = NULL,
	.license = MDP_LICENSE_LGPL_21
};

mdp_render_t mdp_render =
{
	.blit = mdp_render_2xsai_cpp,
	.tag = "2xSaI",
	
	.scale = 2,
	.flags = MDP_RENDER_FLAG_RGB_555to555 |
		 MDP_RENDER_FLAG_RGB_565to565
};

static mdp_func_t mdp_func =
{
	.init = mdp_render_2xsai_init,
	.end = mdp_render_2xsai_end
};

mdp_t mdp =
{
	// Plugin version information.
	.interfaceVersion = MDP_VERSION(1, 0, 0),
	.pluginVersion = MDP_VERSION(1, 0, 0),
	
	// CPU flags.
	.cpuFlagsSupported = MDP_CPUFLAG_X86_MMX,
	.cpuFlagsRequired = MDP_CPUFLAG_X86_MMX,
	
	// UUID: 5c03f2ff-1c20-4f9b-b81c-458de6c7880d
	.uuid = {0x5C, 0x03, 0xF2, 0xFF,
		 0x1C, 0x20,
		 0x4F, 0x9B,
		 0xB8, 0x1C,
		 0x45, 0x8D, 0xE6, 0xC7, 0x88, 0x0D},
	
	// Description.
	.desc = &mdp_desc,
	
	// Functions.
	.func = &mdp_func
};
