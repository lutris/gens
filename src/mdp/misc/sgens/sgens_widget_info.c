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

#include "sgens_widget_info.h"

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

sgens_window_info_widget_t level_info[LEVEL_INFO_COUNT+1] =
{
	{"Score:",	"0",		0, 0, FALSE},
	{"Time:",	"00:00:00",	0, 1, FALSE},
	{"Rings:",	"0",		0, 2, FALSE},
	{"Lives:",	"0",		0, 3, FALSE},
	{"Continues:",	"0",		0, 4, FALSE},
	{"Emeralds:",	"0",		0, 5, FALSE},
	{"Camera X position:",	"0000",	1, 0, FALSE},
	{"Camera Y position:",	"0000",	1, 1, FALSE},
	{"Water level:",	"0000",	1, 3, FALSE},
	{"Rings remaining to get the Perfect Bonus:", "0", 0, 6, TRUE},
	{"Water in Act:",	"OFF",	0, 7, FALSE},
	{NULL, NULL, 0, 0, FALSE}
};

sgens_window_info_widget_t player_info[PLAYER_INFO_COUNT+1] =
{
	{"Angle:",	"0.00°", 0, 0, FALSE},
	{"X position:",	"0000",	 0, 1, FALSE},
	{"Y position:", "0000",	 0, 2, FALSE},
	{NULL, NULL, 0, 0, FALSE}
};
