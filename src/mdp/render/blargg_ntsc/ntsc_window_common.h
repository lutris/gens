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
	const int min;
	const int max;
	const int step;
} ntsc_ctrl_t;

#define NTSC_CTRL_COUNT 10
DLL_LOCAL extern const ntsc_ctrl_t ntsc_controls[NTSC_CTRL_COUNT+1];

#ifdef __cplusplus
}
#endif

#endif /* _MDP_NTSC_WINDOW_COMMON_H */
