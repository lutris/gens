/***************************************************************************
 * Gens: (Win32) Main Window. (Menu Handling Code)                         *
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

#include "gens_menu.hpp"

#include "ui/common/gens/gens_menu_callbacks.hpp"

// C includes.
#include <stdio.h>

// C++ includes.
#include <string>
#include <vector>
using std::string;
using std::vector;

// libgsft includes.
#include "libgsft/gsft_szprintf.h"

// Main menu.
HMENU MainMenu = NULL;
// Accelerator table for the Gens main menu.
HACCEL hAccelTable_Menu = NULL;

// Hash table containing all the menu items.
// Key is the menu ID.
gensMenuMap_t gens_menu_map;


/**
 * gens_menu_parse(): Parse the menu structs.
 * @param menu First item of the array of menu structs to parse.
 * @param container Container to add the menu items to.
 */
void gens_menu_parse(const GensMenuItem_t *menu, HMENU container)
{
	HMENU mnuSubMenu;
	string sMenuText;
	
	// Win32 InsertMenu() parameters.
	unsigned int uFlags;
	UINT_PTR uIDNewItem;
	
	// Vector of accelerators.
	static vector<ACCEL> vAccel;
	ACCEL curAccel;
	
	// If this is the first invokation of Win32_ParseMenu,
	// clear the vector of accelerators and the menu map.
	if (container == MainMenu)
	{
		vAccel.clear();
		gens_menu_map.clear();
	}
	
	while (menu->id != 0)
	{
		// Check what type of menu item this is.
		switch ((menu->flags & GMF_ITEM_MASK))
		{
			case GMF_ITEM_SEPARATOR:
				// Separator.
				uFlags = MF_BYPOSITION | MF_SEPARATOR;
				uIDNewItem = (UINT_PTR)menu->id;
				break;
			
			case GMF_ITEM_SUBMENU:
				// Submenu.
				if (!menu->submenu)
				{
					// No submenu specified. Create a normal menu item for now.
					uFlags = MF_BYPOSITION | MF_STRING;
					uIDNewItem = (UINT_PTR)menu->id;
				}
				else
				{
					// Submenu specified.
					mnuSubMenu = CreatePopupMenu();
					gens_menu_parse(menu->submenu, mnuSubMenu);
					uFlags = MF_BYPOSITION | MF_STRING | MF_POPUP;
					uIDNewItem = (UINT_PTR)mnuSubMenu;
					
					// Add the submenu to the menu map.
					gens_menu_map.insert(gensMenuMapItem_t(menu->id, mnuSubMenu));
				}
				break;
			
			case GMF_ITEM_CHECK:
			case GMF_ITEM_RADIO:
			default:
				// Menu item. (Win32 doesn't treat check or radio items as different types.)
				uFlags = MF_BYPOSITION | MF_STRING;
				uIDNewItem = (UINT_PTR)menu->id;
				break;
		}
		
		// Set the menu text.
		if (menu->text)
			sMenuText = menu->text;
		else
			sMenuText.clear();
		
		// Check for an accelerator.
		if (menu->accelKey != 0)
		{
			// Accelerator specified.
			// TODO: Add the accelerator to the accelerator table.
			curAccel.fVirt = FVIRTKEY | FNOINVERT;
			
			sMenuText += "\t";
			
			// Determine the modifier.
			if (menu->accelModifier & GMAM_CTRL)
			{
				curAccel.fVirt |= FCONTROL;
				sMenuText += "Ctrl+";
			}
			if (menu->accelModifier & GMAM_ALT)
			{
				curAccel.fVirt |= FALT;
				sMenuText += "Alt+";
			}
			if (menu->accelModifier & GMAM_SHIFT)
			{
				curAccel.fVirt |= FSHIFT;
				sMenuText += "Shift+";
			}
			
			// Determine the key.
			// TODO: Add more special keys.
			char tmpKey[8];
			switch (menu->accelKey)
			{
				case GMAK_BACKSPACE:
					curAccel.key = VK_BACK;
					sMenuText += "Backspace";
					break;
				
				case GMAK_ENTER:
					curAccel.key = VK_RETURN;
					sMenuText += "Enter";
					break;
				
				case GMAK_TAB:
					curAccel.key = VK_TAB;
					sMenuText += "Tab";
					break;
				
				case GMAK_F1: case GMAK_F2:  case GMAK_F3:  case GMAK_F4:
				case GMAK_F5: case GMAK_F6:  case GMAK_F7:  case GMAK_F8:
				case GMAK_F9: case GMAK_F10: case GMAK_F11: case GMAK_F12:
					curAccel.key = (menu->accelKey - GMAK_F1) + VK_F1;
					szprintf(tmpKey, sizeof(tmpKey), "F%d", (menu->accelKey - GMAK_F1 + 1));
					
					sMenuText += string(tmpKey);
					break;
					
				default:
					curAccel.key = toupper(menu->accelKey);
					sMenuText += (char)(curAccel.key);
					break;
			}
			
			// Add the accelerator.
			if ((menu->flags & GMF_ITEM_MASK) != GMF_ITEM_SUBMENU)
			{
				curAccel.cmd = menu->id;
				vAccel.push_back(curAccel);
			}
		}
		
		// Add the menu item to the container.
		InsertMenu(container, -1, uFlags, uIDNewItem, sMenuText.c_str());
		
		// Next menu item.
		menu++;
	}
	
	// If this is the first invokation of Win32_ParseMenu, create the accelerator table.
	if (container == MainMenu)
	{
		if (hAccelTable_Menu)
			DestroyAcceleratorTable(hAccelTable_Menu);
		
		// Create the accelerator table.
		hAccelTable_Menu = CreateAcceleratorTable(&vAccel[0], vAccel.size());
		
		// Clear the vector of accelerators.
		vAccel.clear();
	}
}


/**
 * gens_menu_find_item(): Find a submenu in the menu map.
 * @param id Submenu ID.
 * @return Submenu.
 */
HMENU gens_menu_find_item(uint16_t id)
{
	// TODO: Make this a common function.
	
	gensMenuMap_t::iterator mnuIter;
	
	mnuIter = gens_menu_map.find(id & 0xFF00);
	if (mnuIter == gens_menu_map.end())
		return NULL;
	
	return (*mnuIter).second;
}


/**
 * gens_menu_clear(): Clear the menu map.
 */
void gens_menu_clear(void)
{
	gens_menu_map.clear();
}
