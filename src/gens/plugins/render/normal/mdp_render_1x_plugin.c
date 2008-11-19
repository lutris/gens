/***************************************************************************
 * Gens: [MDP] 1x renderer. (Plugin Data File)                             *
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

#include "plugins/plugin.h"
#include <string.h>
#include <stdint.h>

#include "mdp_render_1x.hpp"

static MDP_Desc_t MDP_Desc =
{
	.name = "Normal Renderer",
	.author_mdp = "David Korth",
	.author_orig = "Stéphane Dallongeville",
	.description = "Normal 1x renderer."
};

static MDP_Render_t MDP_Render =
{
	.blit = mdp_render_1x_cpp,
	.scale = 1,
	.tag = "Normal"
};

MDP_t mdp_render_1x =
{
	.interfaceVersion = MDP_INTERFACE_VERSION,
	.pluginVersion = MDP_VERSION(0, 0, 1),
	.type = MDPT_RENDER,
	.desc = &MDP_Desc,
	
	// Init/Shutdown functions
	.init = NULL,
	.end = NULL,
	
	.plugin_t = (void*)&MDP_Render
};
