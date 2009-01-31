/***************************************************************************
 * Gens: [MDP] VDP Layer Options. (Options Definitions)                    *
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

#include "vlopt_options.h"

// Contains VDP Layer Options definitions.
#include "mdp/mdp_host.h"

// Array of VDP layer options.
const vlopt_option vlopt_options[] =
{
	{"Scroll A Low",		MDP_VDP_LAYER_OPTIONS_SCROLLA_LOW},
	{"Scroll A High",		MDP_VDP_LAYER_OPTIONS_SCROLLA_HIGH},
	{"Scroll A Swap Priority",	MDP_VDP_LAYER_OPTIONS_SCROLLA_SWAP},
	{"Scroll B Low",		MDP_VDP_LAYER_OPTIONS_SCROLLB_LOW},
	{"Scroll B High"	,	MDP_VDP_LAYER_OPTIONS_SCROLLB_HIGH},
	{"Scroll B Swap Priority",	MDP_VDP_LAYER_OPTIONS_SCROLLB_SWAP},
	{"Sprite Low",			MDP_VDP_LAYER_OPTIONS_SPRITE_LOW},
	{"Sprite High"		,	MDP_VDP_LAYER_OPTIONS_SPRITE_HIGH},
	{"Sprite Swap Priority",	MDP_VDP_LAYER_OPTIONS_SPRITE_SWAP},
	{"Sprite Always On Top",	MDP_VDP_LAYER_OPTIONS_SPRITE_ALWAYSONTOP},
	{"Palette Lock",		MDP_VDP_LAYER_OPTIONS_PALETTE_LOCK},
	{0, 0}
};
