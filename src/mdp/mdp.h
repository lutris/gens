/***************************************************************************
 * MDP: Mega Drive Plugins - Base Interface Definitions.                   *
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

#ifndef __MDP_H
#define __MDP_H

#include <stdint.h>

#include "mdp_fncall.h"
#include "mdp_version.h"

/* MDP interface version. */
#define MDP_INTERFACE_VERSION MDP_VERSION(1, 0, 0)

/* Licenses. */
#define MDP_LICENSE_GPL_2	"GPL-2"
#define MDP_LICENSE_GPL_3	"GPL-3"
#define MDP_LICENSE_LGPL_2	"LGPL-2"
#define MDP_LICENSE_LGPL_21	"LGPL-2.1"
#define MDP_LICENSE_LGPL_3	"LGPL-3"
#define MDP_LICENSE_BSD		"BSD"
#define MDP_LICENSE_PD		"Public Domain"

#ifdef __cplusplus
extern "C" {
#endif

/* Plugin description struct. */
#pragma pack(1)
typedef struct PACKED _mdp_desc_t
{
	const char* name;
	const char* author_mdp;
	const char* author_orig;
	const char* description;
	const char* website;
	const char* license;
	
	/* Filler for alignment purposes. */
	const void* reserved1;
	const void* reserved2;
	
	/* Icon data. (PNG format) */
	const unsigned char* icon;
	const unsigned int iconLength;
} mdp_desc_t;
#pragma pack()

/* MDP Host Services. */
struct _mdp_t;
#include "mdp_host.h"

/* Plugin functions. */
typedef int (MDP_FNCALL *mdp_init_fn)(const mdp_host_t *host_srv);
typedef int (MDP_FNCALL *mdp_end_fn)(void);

/* Plugin function struct. */
#pragma pack(1)
typedef struct PACKED _mdp_func_t
{
	// Init/Shutdown functions
	mdp_init_fn	init;
	mdp_end_fn	end;
} mdp_func_t;
#pragma pack()

/* Main MDP plugin struct. */
#pragma pack(1)
typedef struct PACKED _mdp_t
{
	// Plugin version information.
	const uint32_t interfaceVersion;
	const uint32_t pluginVersion;
	
	// CPU flags.
	const uint32_t cpuFlagsSupported;
	const uint32_t cpuFlagsRequired;
	
	// UUID - each plugin must have a unique ID.
	const unsigned char uuid[16];
	
	// Description.
	mdp_desc_t *desc;
	
	// Functions.
	mdp_func_t *func;
} mdp_t;
#pragma pack()

#ifdef __cplusplus
}
#endif

#endif /* __MDP_H */
