/***************************************************************************
 * Gens: MDP: Mega Drive Plugin - Host Services.                           *
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

#ifndef GENS_MDP_HOST_GENS_MENU_HPP
#define GENS_MDP_HOST_GENS_MENU_HPP

#ifdef __cplusplus
extern "C" {
#endif

#include "mdp_host_gens.h"

// Host Services functions. (Menus)

int MDP_FNCALL mdp_host_menu_item_add(mdp_t *plugin, mdp_menu_handler_fn handler,
				      int menu_id, const char *text);
int MDP_FNCALL mdp_host_menu_item_remove(mdp_t *plugin, int menu_item_id);

int MDP_FNCALL mdp_host_menu_item_set_text(mdp_t *plugin, int menu_item_id, const char *text);
int MDP_FNCALL mdp_host_menu_item_get_text(mdp_t *plugin, int menu_item_id,
					   char *text_buf, unsigned int size);

int MDP_FNCALL mdp_host_menu_item_set_checked(mdp_t *plugin, int menu_item_id, int checked);
int MDP_FNCALL mdp_host_menu_item_get_checked(mdp_t *plugin, int menu_item_id);

#ifdef __cplusplus
}
#endif

#endif /* GENS_MDP_HOST_GENS_MENU_HPP */
