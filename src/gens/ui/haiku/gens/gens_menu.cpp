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

#include "gens_menu.hpp"
#include "ui/common/gens/gens_menu_callbacks.hpp"

// Message logging.
#include "macros/log_msg.h"

// C includes.
#include <stdlib.h>
#include <string.h>

// C++ includes.
#include <string>
using std::string;

// Set to 0 to temporarily disable menu callbacks.
int gens_menu_do_callbacks = 1;

// Hash table containing all the menu items.
// Key is the menu ID.
gensMenuMap_t gens_menu_map;

