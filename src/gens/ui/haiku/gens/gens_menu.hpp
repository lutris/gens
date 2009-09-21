/***************************************************************************
 * Gens: (Haiku) Main Window. (Menu Handling Code)                         *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2009 by David Korth                                  *
 * Copyright (c) 2009 by Phil Costin                                       *
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

#ifndef GENS_HAIKU_MENU_HPP
#define GENS_HAIKU_MENU_HPP

#include "ui/common/gens/gens_menu.h"
#include <InterfaceKit.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int gens_menu_do_callbacks;

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

void gens_menu_parse(const GensMenuItem_t* menu, BMenuBar *menuBar, BWindow *win);

// Hash table containing all the menu items.
// Key is the menu ID.
#include "libgsft/gsft_hashtable.hpp"
#include <utility>
typedef GSFT_HASHTABLE<uint16_t, BMenuItem*> gensMenuMap_t;
typedef std::pair<uint16_t, BMenuItem*> gensMenuMapItem_t;

extern gensMenuMap_t gens_menu_map;

#endif /* __cplusplus */

#endif /* GENS_HAIKU_MENU_HPP */
