/*****************************************************************************
 * MDP: Super 2xSaI renderer. (Plugin Data File)                             *
 *                                                                           *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                         *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                                *
 * Copyright (c) 2008-2009 by David Korth                                    *
 * Super 2xSaI Copyright (c) by Derek Liauw Kie Fa and Robert J. Ohannessian *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify it   *
 * under the terms of the GNU Lesser General Public License as published     *
 * by the Free Software Foundation; either version 2.1 of the License, or    *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This program is distributed in the hope that it will be useful, but       *
 * WITHOUT ANY WARRANTY; without even the implied warranty of                *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License along   *
 * with this program; if not, write to the Free Software Foundation, Inc.,   *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.             *
 *****************************************************************************/

#include "mdp_render_super_2xsai.h"

#include "mdp/mdp.h"
#include "mdp/mdp_cpuflags.h"

static mdp_desc_t mdp_desc =
{
	.name = "Super 2xSaI Renderer",
	.author_mdp = "David Korth",
	.author_orig = "Derek Liauw Kie Fa and Robert J. Ohannessian",
	.description = "Super 2x Scale and Interpolation renderer.",
	.website = NULL,
	.license = MDP_LICENSE_LGPL_21
};

mdp_render_t mdp_render =
{
	.blit = mdp_render_super_2xsai_cpp,
	.tag = "Super 2xSaI",
	
	.scale = 2,
	.flags = MDP_RENDER_FLAG_RGB_555to555 |
		 MDP_RENDER_FLAG_RGB_565to565
};

static mdp_func_t mdp_func =
{
	.init = mdp_render_super_2xsai_init,
	.end = mdp_render_super_2xsai_end
};

mdp_t mdp =
{
	// Plugin version information.
	.interfaceVersion = MDP_VERSION(1, 0, 0),
	.pluginVersion = MDP_VERSION(1, 0, 0),
	
	// CPU flags.
	.cpuFlagsSupported = MDP_CPUFLAG_X86_MMX,
	.cpuFlagsRequired = MDP_CPUFLAG_X86_MMX,
	
	// UUID: 7f72d9ea-340c-470c-999b-02b92ebeee70
	.uuid = {0x7F, 0x72, 0xD9, 0xEA,
		 0x34, 0x0C,
		 0x47, 0x0C,
		 0x99, 0x9B,
		 0x02, 0xB9, 0x2E, 0xBE, 0xEE, 0x70},
	
	// Description.
	.desc = &mdp_desc,
	
	// Functions.
	.func = &mdp_func
};
