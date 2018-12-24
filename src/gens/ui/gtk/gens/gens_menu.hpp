/***************************************************************************
 * Gens: (GTK+) Main Window. (Menu Handling Code)                          *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2009 by David Korth                                  *
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

#ifndef GENS_GTK_MENU_HPP
#define GENS_GTK_MENU_HPP

#include "ui/common/gens/gens_menu.h"
#include <gtk/gtk.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int	gens_menu_do_callbacks;

void		gens_menu_parse(const GensMenuItem_t* menu, GtkWidget *container, GtkAccelGroup *accel_group);

GtkWidget*	gens_menu_find_item(uint16_t id);
void		gens_menu_clear(void);

void		gens_gtk_menu_callback(GtkMenuItem *menuitem, gpointer user_data);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

// Hashtable containing all the menu items.
// Key is the menu ID.
#include "libgsft/gsft_hashtable.hpp"
#include <utility>
typedef GSFT_HASHTABLE<uint16_t, GtkWidget*> gensMenuMap_t;
typedef std::pair<uint16_t, GtkWidget*> gensMenuMapItem_t;

extern gensMenuMap_t gens_menu_map;

#endif /* __cplusplus */

#endif /* GENS_GTK_MENU_HPP */
