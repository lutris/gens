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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "sgens_widget_info.h"
#include "sgens.h"

// MDP memory functions.
#include "mdp/mdp_mem.h"

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
	{"Angle:",	"0.00" DEGREE_SYMBOL, 0, 0, FALSE},
	{"X position:",	"0000",	 0, 1, FALSE},
	{"Y position:", "0000",	 0, 2, FALSE},
	{NULL, NULL, 0, 0, FALSE}
};


/**
 * sgens_get_widget_info(): Get the current widget info.
 * @param info Struct to return widget info in.
 */
void MDP_FNCALL sgens_get_widget_info(sgens_widget_info *info)
{
	if (sgens_current_rom_type <= SGENS_ROM_TYPE_UNSUPPORTED)
		return;
	
	// Values common to all supported Sonic games.
	
	// Score.
	sgens_host_srv->mem_read_32(MDP_MEM_MD_RAM, 0xFE26, &info->score);
	info->score *= 10;
	
	// Time.
	sgens_host_srv->mem_read_8(MDP_MEM_MD_RAM, 0xFE23, &info->time.min);
	sgens_host_srv->mem_read_8(MDP_MEM_MD_RAM, 0xFE24, &info->time.sec);
	sgens_host_srv->mem_read_8(MDP_MEM_MD_RAM, 0xFE25, &info->time.frames);
	
	// Rings.
	sgens_host_srv->mem_read_16(MDP_MEM_MD_RAM, 0xFE20, &info->rings);
	
	// Lives.
	sgens_host_srv->mem_read_8(MDP_MEM_MD_RAM, 0xFE12, &info->lives);
	
	// Continues.
	sgens_host_srv->mem_read_8(MDP_MEM_MD_RAM, 0xFE18, &info->continues);
	
	// Rings remaining for Perfect Bonus.
	// This is only applicable for Sonic 2.
	if (sgens_current_rom_type >= SGENS_ROM_TYPE_SONIC2_REV00 &&
	    sgens_current_rom_type <= SGENS_ROM_TYPE_SONIC2_REV02)
	{
		sgens_host_srv->mem_read_16(MDP_MEM_MD_RAM, 0xFF40, &info->rings_for_perfect_bonus);
	}
	
	// Water level.
	sgens_host_srv->mem_read_16(MDP_MEM_MD_RAM, 0xF648, &info->water_level);
	
	// TODO: Camera position and player position don't seem to be working
	// correctly with Sonic 3, S&K, etc.
	
	uint8_t angle_bytes[2];
	uint16_t angle;
	
	if (sgens_current_rom_type >= SGENS_ROM_TYPE_SONIC1_REV00 &&
	    sgens_current_rom_type <= SGENS_ROM_TYPE_SONIC1_REVXB)
	{
		// S1-specific information.
		
		// Number of emeralds.
		sgens_host_srv->mem_read_8(MDP_MEM_MD_RAM, 0xFE57, &info->emeralds);
		
		// Camera X position.
		sgens_host_srv->mem_read_16(MDP_MEM_MD_RAM, 0xFF10, &info->camera_x);
		
		// Camera Y position.
		sgens_host_srv->mem_read_16(MDP_MEM_MD_RAM, 0xFF14, &info->camera_y);
		
		// Player angle.
		sgens_host_srv->mem_read_8(MDP_MEM_MD_RAM, 0xD026, &angle_bytes[0]);
		sgens_host_srv->mem_read_8(MDP_MEM_MD_RAM, 0xD027, &angle_bytes[1]);
		
		angle = (angle_bytes[0] | ((uint16_t)angle_bytes[1] << 8));
		info->player_angle = ((double)(angle) * 1.40625);
		
		// Player X position.
		sgens_host_srv->mem_read_16(MDP_MEM_MD_RAM, 0xD008, &info->player_x);
		
		// Player Y position.
		sgens_host_srv->mem_read_16(MDP_MEM_MD_RAM, 0xD00C, &info->player_y);
	}
	else
	{
		// Information for games other than S1.
		
		// Number of emeralds.
		sgens_host_srv->mem_read_8(MDP_MEM_MD_RAM, 0xFEB1, &info->emeralds);
		
		// Camera X position.
		sgens_host_srv->mem_read_16(MDP_MEM_MD_RAM, 0xEE00, &info->camera_x);
		
		// Camera Y position.
		sgens_host_srv->mem_read_16(MDP_MEM_MD_RAM, 0xEE04, &info->camera_y);
		
		// Player angle.
		sgens_host_srv->mem_read_8(MDP_MEM_MD_RAM, 0xB026, &angle_bytes[0]);
		sgens_host_srv->mem_read_8(MDP_MEM_MD_RAM, 0xB027, &angle_bytes[1]);
		
		angle = (angle_bytes[0] | ((uint16_t)angle_bytes[1] << 8));
		info->player_angle = ((double)(angle) * 1.40625);
		
		// Player X position.
		sgens_host_srv->mem_read_16(MDP_MEM_MD_RAM, 0xB008, &info->player_x);
		
		// Player Y position.
		sgens_host_srv->mem_read_16(MDP_MEM_MD_RAM, 0xB00C, &info->player_y);
	}
}
