/***************************************************************************
 * Gens: [MDP] Sonic Gens. (Window Code) (Information)                     *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * SGens Copyright (c) 2002 by LOst                                        *
 * MDP port Copyright (c) 2008-2009 by David Korth                         *
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

#ifndef _MDP_MISC_SGENS_WINDOW_INFO_H
#define _MDP_MISC_SGENS_WINDOW_INFO_H

#include "mdp/mdp_fncall.h"
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _sgens_window_info_widget_t
{
	const char* description;	// Description.
	const char* initial;		// Initial value. (If NULL, default to "0".)
	int column;			// Column. (0 == left; 1 == right)
	int row;			// Row.
	int fill_all_cols;		// If non-zero, fills all columns.
} sgens_window_info_widget_t;

#define LEVEL_INFO_COUNT 11
DLL_LOCAL extern sgens_window_info_widget_t level_info[LEVEL_INFO_COUNT+1];
typedef enum _LEVEL_INFO_ID
{
	LEVEL_INFO_SCORE	= 0,
	LEVEL_INFO_TIME		= 1,
	LEVEL_INFO_RINGS	= 2,
	LEVEL_INFO_LIVES	= 3,
	LEVEL_INFO_CONTINUES	= 4,
	LEVEL_INFO_EMERALDS	= 5,
	LEVEL_INFO_CAMERA_X	= 6,
	LEVEL_INFO_CAMERA_Y	= 7,
	LEVEL_INFO_WATER_LEVEL	= 8,
	LEVEL_INFO_RINGS_PERFECT = 9,
	LEVEL_INFO_WATER_ENABLED = 10,
} LEVEL_INFO_ID;

#define PLAYER_INFO_COUNT 3
DLL_LOCAL extern sgens_window_info_widget_t player_info[PLAYER_INFO_COUNT+1];
typedef enum _PLAYER_INFO_ID
{
	PLAYER_INFO_ANGLE	= 0,
	PLAYER_INFO_X		= 1,
	PLAYER_INFO_Y		= 2,
} PLAYER_INFO_ID;

#ifdef __cplusplus
}
#endif

#endif /* _MDP_MISC_SGENS_WINDOW_INFO_H */
