/***************************************************************************
 * Gens: (Haiku) Main Window.                                              *
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

#include "gens_window.h"
#include "gens_window_callbacks.hpp"
#include "gens_window_sync.hpp"

#include "emulator/g_main.hpp"

// Menus.
#include "gens_menu.hpp"
#include "ui/common/gens/gens_menu.h"

// Message logging.
#include "macros/log_msg.h"

// BeOS menu includes.
#include <Menu.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <Message.h>
#include <View.h>


/**
 * gens_window_create(): Create the Gens window.
 */
void gens_window_create(void)
{
}

/**
 * gens_window_create_menubar(): Create the Gens Menu Bar
 */
void gens_window_create_menubar(void)
{
}


GensWindow::GensWindow()
	  : BWindow(BRect(10, 10, 400, 300), "Gens/GS", B_TITLED_WINDOW, B_NOT_RESIZABLE | B_ASYNCHRONOUS_CONTROLS | B_QUIT_ON_WINDOW_CLOSE)
{
	// Create the menu bar.
	BMenuBar *menuBar = new BMenuBar(Bounds(), "menu");
	AddChild(menuBar);
	
	// Create the menus.
	gens_menu_parse(&gmiMain[0], menuBar, this);
}


GensWindow::~GensWindow()
{
}
