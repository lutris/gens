/***************************************************************************
 * Gens: Plugin interface definition.                                      *
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

#ifndef GENS_PLUGIN_H
#define GENS_PLUGIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>


// Version number macro.
#define GP_VERSION(major, minor, revision) \
	((((major) & 0xFF) << 24) | (((minor) & 0xFF) << 16) | ((revision) & 0xFFFF))

// Gens Plugin interface version.
#define GENSPLUGIN_VERSION GP_VERSION(0, 0, 1)


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


// Plugin description struct.
typedef struct
{
	const char name[256];
	const char author[256];
	const char description[1024];
} GensPlugin_Desc_t;


// Plugin types.
enum GensPlugin_Type
{
	GENSPLUGIN_RENDER,
};


// Gens rendering info
typedef struct
{
	// MD screen buffers.
	uint16_t *screen16;
	uint32_t *screen32;
	
	// Current bpp.
	uint8_t bpp;
	
	// Output screen buffer parameters.
	void *screen;
	int x;
	int y;
	int pitch;
	int offset;
} GensPlugin_Render_Info_t;


// Render plugin definition.
typedef void (GENS_FNCALL *BlitFn)(GensPlugin_Render_Info_t *renderInfo);
typedef struct
{
	// Blit function.
	BlitFn blit;
	
	// Scaling ratio. (1 == 320x240; 2 = 640x480; etc)
	const unsigned int scale;
	
	// Render tag.
	const char tag[64];
} GensPlugin_Render_t;


// Basic plugin definition struct.
typedef struct
{
	// Plugin interface version.
	const uint32_t interfaceVersion;
	
	// Version of this plugin.
	const uint32_t pluginVersion;
	
	// Type of plugin.
	const uint32_t type;
	
	// Description struct.
	const GensPlugin_Desc_t *desc;
	
	// Struct definition for the specified plugin type.
	const void *plugin_t;
} GensPlugin_t;


#ifdef __cplusplus
}
#endif

#endif /* GENS_PLUGIN_H */
