/***************************************************************************
 * Gens: MDP: Mega Drive Plugin - Base Interface Definitions.              *
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

#ifndef GENS_MDP_H
#define GENS_MDP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>


// Version number macro.
#define MDP_VERSION(major, minor, revision) \
	((((major) & 0xFF) << 24) | (((minor) & 0xFF) << 16) | ((revision) & 0xFFFF))

// Gens Plugin interface version.
#define MDP_INTERFACE_VERSION MDP_VERSION(0, 0, 1)


// Function pointer calling conventions from SDL's begin_code.h
// TODO: Combine this with the #defines from compress.h
#ifndef GENS_FNCALL
#if defined(__WIN32__) && !defined(__GNUC__)
#define GENS_FNCALL __cdecl
#else
#ifdef __OS2__
#define GENS_FNCALL _System
#else
#define GENS_FNCALL
#endif
#endif
#endif


// Licenses
#define MDP_LICENSE_GPL_2	"GPL-2"
#define MDP_LICENSE_GPL_3	"GPL-3"
#define MDP_LICENSE_BSD		"BSD"

// Plugin description struct.
typedef struct
{
	const char* name;
	const char* author_mdp;
	const char* author_orig;
	const char* description;
	const char* website;
	const char* license;
} MDP_Desc_t;


// Plugin types.
enum MDP_Type
{
	MDPT_NULL = 0,
	MDPT_RENDER,
};


// Basic plugin definition struct.
typedef void (GENS_FNCALL *mdp_init)(void);
typedef void (GENS_FNCALL *mdp_end)(void);

typedef struct
{
	const uint32_t interfaceVersion;
	const uint32_t pluginVersion;
	const uint32_t type;
	const MDP_Desc_t *desc;
	
	// CPU flag information.
	const uint32_t cpuFlagsSupported;
	const uint32_t cpuFlagsRequired;
	
	// Init/Shutdown functions
	mdp_init init;
	mdp_end end;
	
	// Struct definition for the specified plugin type.
	const void *plugin_t;
} MDP_t;


#ifdef __cplusplus
}
#endif

#endif /* GENS_MDP_H */
