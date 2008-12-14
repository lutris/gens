/***************************************************************************
 * Gens: [MDP] Game Genie.                                                 *
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

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "mdp_misc_game_genie.hpp"
#include "mdp_misc_game_genie_plugin.h"

// MDP includes.
#include "mdp/mdp_cpuflags.h"
#include "mdp/mdp_error.h"

// MDP Host Services.
static MDP_Host_t *gg_host_srv = NULL;
static int gg_menuItemID = 0;


/**
 * mdp_misc_game_genie_init(): Initialize the Game Genie plugin.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_misc_game_genie_init(MDP_Host_t *host_srv)
{
	// Save the MDP Host Services pointer.
	gg_host_srv = host_srv;
	
	// Create a menu item.
	gg_menuItemID = gg_host_srv->menu_item_add(&mdp, &mdp_misc_game_genie_menu_handler, 0, "&Game Genie");
	printf("Game Genie plugin initialized. Menu item ID: 0x%04X\n", gg_menuItemID);
	
	// Initialized.
	return MDP_ERR_OK;
}


/**
 * mdp_misc_game_genie_end(): Shut down the Game Genie plugin.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_misc_game_genie_end(void)
{
	if (!gg_host_srv)
		return MDP_ERR_OK;
	
	// Remove the menu item.
	gg_host_srv->menu_item_remove(&mdp, gg_menuItemID);
	
	// Plugin is shut down.
	return MDP_ERR_OK;
}


/**
 * mdp_misc_game_genie_menu_handler(): Menu handler function.
 * @param menu_item_id Menu item ID.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_misc_game_genie_menu_handler(int menu_item_id)
{
	printf("Game Genie Menu Handler: Menu item 0x%04X\n", menu_item_id);
	return MDP_ERR_OK;
}
