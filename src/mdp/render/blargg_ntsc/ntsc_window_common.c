/***************************************************************************
 * MDP: Blargg's NTSC renderer. (Window Code) (Common Data)                *
 *                                                                         *
 * Copyright (c) 2006 by Shay Green                                        *
 * MDP version Copyright (c) 2008-2009 by David Korth                      *
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

#include "ntsc_window_common.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef GENS_UI_GTK
#define NTSC_MNEMONIC_CHAR "_"
#else
#define NTSC_MNEMONIC_CHAR "&"
#endif

// Presets.
const ntsc_preset_t ntsc_presets[NTSC_PRESETS_COUNT+1] =
{
	{"Composite",	&md_ntsc_composite},
	{"S-Video",	&md_ntsc_svideo},
	{"RGB",		&md_ntsc_rgb},
	{"Monochrome",	&md_ntsc_monochrome},
	{"Custom",	NULL},
	{NULL, NULL}
};

// Adjustment controls.
const ntsc_ctrl_t ntsc_controls[NTSC_CTRL_COUNT+1] =
{
	{NTSC_MNEMONIC_CHAR "Hue",			-180, 180, 1},
	{NTSC_MNEMONIC_CHAR "Saturation",		0, 200, 5},
	{NTSC_MNEMONIC_CHAR "Contrast",			-100, 100, 5},
	{NTSC_MNEMONIC_CHAR "Brightness",		-100, 100, 5},
	{"S" NTSC_MNEMONIC_CHAR "harpness",		-100, 100, 5},
	
	// "Advanced" parameters.
	{NTSC_MNEMONIC_CHAR "Gamma",			50, 150, 5},
	{NTSC_MNEMONIC_CHAR "Resolution",		-100, 100, 5},
	{NTSC_MNEMONIC_CHAR "Artifacts",		-100, 100, 5},
	{"Color " NTSC_MNEMONIC_CHAR "Fringing",	-100, 100, 5},
	{"Color B" NTSC_MNEMONIC_CHAR "leed",		-100, 100, 5},
	
	{NULL, 0, 0, 0}
};
