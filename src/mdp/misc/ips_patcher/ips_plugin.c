/***************************************************************************
 * MDP: IPS Patcher. (Plugin Data File)                                    *
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

#include "ips.h"
#include "ips_icon.h"

static mdp_desc_t mdp_desc =
{
	.name = "IPS Patcher",
	.author_mdp = "David Korth",
	.author_orig = "Stéphane Dallongeville",
	.description = "Applies IPS patches to ROMs.\nIcon provided by SDM.",
	.website = NULL,
	.license = MDP_LICENSE_GPL_2,
	
	.icon = ips_icon,
	.iconLength = sizeof(ips_icon)
};

static mdp_func_t mdp_func =
{
	.init = ips_init,
	.end = ips_end
};

mdp_t mdp =
{
	// Plugin version information.
	.interfaceVersion = MDP_INTERFACE_VERSION,
	.pluginVersion = MDP_VERSION(0, 9, 0),
	
	// CPU flags.
	.cpuFlagsSupported = 0,
	.cpuFlagsRequired = 0,
	
	// UUID: c5223a2d-a525-4216-98fc-836879e251c3
	.uuid = {0xC5, 0x22, 0x3A, 0x2D,
		 0xA5, 0x25,
		 0x42, 0x16,
		 0x98, 0xFC,
		 0x83, 0x68, 0x79, 0xE2, 0x51, 0xC3},
	
	// Description.
	.desc = &mdp_desc,
	
	// Functions.
	.func = &mdp_func
};
