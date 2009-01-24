/***************************************************************************
 * Gens: Main Window - Synchronization Functions.                          *
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gens_window_sync.h"
#include "gens_menu.h"

// C includes.
#include <string.h>

const char* const gws_rom_format_prefix[6] = {"[----]", "[MD]", "[32X]", "[SCD]", "[SCDX]", NULL};

#ifdef GENS_OPENGL
// OpenGL resolutions mapped to menu IDs.
const uint32_t gws_opengl_resolutions[][2] =
{
	{GENS_GWS_RES(320, 240),		IDM_GRAPHICS_OPENGL_RES_320x240},
	{GENS_GWS_RES(640, 480),		IDM_GRAPHICS_OPENGL_RES_640x480},
	{GENS_GWS_RES(800, 600),		IDM_GRAPHICS_OPENGL_RES_800x600},
	{GENS_GWS_RES(960, 720),		IDM_GRAPHICS_OPENGL_RES_960x720},
	{GENS_GWS_RES(1024, 768),		IDM_GRAPHICS_OPENGL_RES_1024x768},
	{GENS_GWS_RES(1280, 960),		IDM_GRAPHICS_OPENGL_RES_1280x960},
	{GENS_GWS_RES(1280, 1024),		IDM_GRAPHICS_OPENGL_RES_1280x1024},
	{GENS_GWS_RES(1400, 1050),		IDM_GRAPHICS_OPENGL_RES_1400x1050},
	{GENS_GWS_RES(1600, 1200),		IDM_GRAPHICS_OPENGL_RES_1600x1200},
	{0, 0},
};
#endif /* GENS_OPENGL */

#ifdef GENS_DEBUGGER
const char* const gws_debug_items[] =
{
	"&Genesis - 68000",
	"Genesis - &Z80",
	"Genesis - &VDP",
	"&SegaCD - 68000",
	"SegaCD - &CDC",
	"SegaCD - GF&X",
	"32X - Main SH2",
	"32X - Sub SH2",
	"32X - VDP",
	NULL
};
#endif /* GENS_DEBUGGER */
