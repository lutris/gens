/***************************************************************************
 * MDP: Blargg's NTSC Filter. (Window Code) (Common Data)                  *
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

#ifndef _MDP_NTSC_WINDOW_COMMON_H
#define _MDP_NTSC_WINDOW_COMMON_H

#include "mdp/mdp_fncall.h"

#include "md_ntsc.hpp"

#ifdef _WIN32
#define NTSC_DEGREE_SYMBOL "\xB0"
#else
#define NTSC_DEGREE_SYMBOL "°"
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Presets.
typedef struct _ntsc_preset_t
{
	const char *name;
	const md_ntsc_setup_t *setup;
} ntsc_preset_t;

#define NTSC_PRESETS_COUNT 5
DLL_LOCAL extern const ntsc_preset_t ntsc_presets[NTSC_PRESETS_COUNT+1];

// Adjustment controls.
typedef struct _ntsc_ctrl_t
{
	const char *name;
	const char *name_mnemonic;
	const int min;
	const int max;
	const int step;
} ntsc_ctrl_t;

#define NTSC_CTRL_COUNT 10
DLL_LOCAL extern const ntsc_ctrl_t ntsc_controls[NTSC_CTRL_COUNT+1];

// Values.
typedef enum _ntsc_value_t
{
	NTSC_VALUE_HUE		= 0,
	NTSC_VALUE_SATURATION	= 1,
	NTSC_VALUE_CONTRAST	= 2,
	NTSC_VALUE_BRIGHTNESS 	= 3,
	NTSC_VALUE_SHARPNESS	= 4,
	NTSC_VALUE_GAMMA	= 5,
	NTSC_VALUE_RESOLUTION	= 6,
	NTSC_VALUE_ARTIFACTS	= 7,
	NTSC_VALUE_FRINGING	= 8,
	NTSC_VALUE_BLEED	= 9,
	
	NTSC_VALUE_MAX
} ntsc_value_t;

DLL_LOCAL int MDP_FNCALL ntsc_internal_to_display(ntsc_value_t valID, double ntsc_val);
DLL_LOCAL double MDP_FNCALL ntsc_display_to_internal(ntsc_value_t valID, int disp_val);

#ifdef __cplusplus
}
#endif

#endif /* _MDP_NTSC_WINDOW_COMMON_H */
