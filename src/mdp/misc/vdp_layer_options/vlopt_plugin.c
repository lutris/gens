/***************************************************************************
 * Gens: [MDP] VDP Layer Options. (Plugin Data File)                       *
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

#include "vlopt.hpp"
#include "vlopt_icon.h"

static MDP_Desc_t MDP_Desc =
{
	.name = "VDP Layer Options",
	.author_mdp = "David Korth",
	.author_orig = NULL,
	.description = "VDP Layer Options adjustment plugin.\nIcon provided by SkyLights.",
	.website = NULL,
	.license = MDP_LICENSE_GPL_2,
	
	.icon = vlopt_icon,
	.iconLength = sizeof(vlopt_icon)
};

static MDP_Func_t MDP_Func =
{
	.init = vlopt_init,
	.end = vlopt_end
};

MDP_t mdp =
{
	// Plugin version information.
	.interfaceVersion = MDP_INTERFACE_VERSION,
	.pluginVersion = MDP_VERSION(0, 2, 0),
	
	// CPU flags.
	.cpuFlagsSupported = 0,
	.cpuFlagsRequired = 0,
	
	// UUID: e383912e-105f-44d1-a23f-35838bc3178d
	.uuid = {0xE3, 0x83, 0x91, 0x2E,
		 0x10, 0x5f,
		 0x44, 0xD1,
		 0xA2, 0x3F,
		 0x35, 0x83, 0x8B, 0xC3, 0x17, 0x8D},
	
	// Description.
	.desc = &MDP_Desc,
	
	// Functions.
	.func = &MDP_Func
};
