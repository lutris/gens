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

// GTK+ includes.
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

// GTK+ misc. (NOTE: DEPRECATED)
#include "gtk-misc.h"

// Set to 0 to temporarily disable menu callbacks.
int gens_menu_do_callbacks = 1;

// Hash table containing all the menu items.
// Key is the menu ID.
gensMenuMap_t gens_menu_map;

// Menu icons.
// See ui/common/gens/gens_menu.h:IDIM_* for the icon constants.

// gens_gtk_menu_icon_t:
// - first: bool - true if stock icon; false if filename.
// - second: stock icon name or filename, depending on first.
typedef struct _gens_gtk_menu_icon_t
{
	bool		stock;
	const char*	name;
} gens_gtk_menu_icon_t;

static const gens_gtk_menu_icon_t gens_gtk_menu_icons[] =
{
	{false,	NULL},	// IDIM_MENU_ICON == 0x0100 == not used
	{false, "gens_16x16.png"},
	{false, "gensgs_16x16.png"},
	{true,	"gtk-open"},
	{true,	"gtk-save"},
	{true,	"gtk-save-as"},
	{true,	"gtk-refresh"},
	{true,	"gtk-revert-to-saved"},
	{true,	"gtk-close"},
	{true,	"gtk-quit"},
	{true,	"gtk-cdrom"},
	{true,	"gtk-cdrom"},
	{false,	"modem.png"},
	{false,	"chronometer.png"},
	{false,	"dialog-password.png"},
	{true,	"gtk-fullscreen"},
	{true,	"gtk-select-color"},
	{false,	"viewmag.png"},
	{false,	"2rightarrow.png"},
	{true,	"gtk-copy"},
	{true,	"gtk-refresh"},
	{false,	"preferences-system.png"},
	{false,	"preferences-desktop-gaming.png"},
	{false,	"document-open-folder.png"},
	{false,	"binary.png"},
	{false,	"memory.png"},
	{true,	"gtk-help"},
	{false,	"kbugbuster.png"},
	{false,	NULL},	// End of array.
};


/**
 * gens_menu_parse(): Parse the menu structs.
 * @param menu First item of the array of menu structs to parse.
 * @param container Container to add the menu items to.
 * @param accel_group Accelerator group to add accelerators to.
 */
void gens_menu_parse(const GensMenuItem_t* menu, GtkWidget *container, GtkAccelGroup *accel_group)
{
	GtkWidget *mnuItem, *subMenu;
	GtkWidget *icon;
	bool bMenuHasIcon;
	GSList *radioGroup = NULL;
	bool bSetCallbackHandler;
	
	// Menu text.
	string sMenuText;
	size_t mnemonicPos;
	const char* sMenuText_cstr;
	
	while (menu->id != 0)
	{
		// Convert the Win32/Qt mnemonic symbol ("&") to the GTK+ mnemonic symbol ("_").
		if (menu->text)
		{
			// Menu text specified.
			sMenuText = string(menu->text);
			mnemonicPos = sMenuText.find('&');
			if (mnemonicPos != string::npos)
				sMenuText[mnemonicPos] = '_';
		}
		else
		{
			// No menu text.
			sMenuText.clear();
		}
		
		if (sMenuText.empty())
			sMenuText_cstr = NULL;
		else
			sMenuText_cstr = sMenuText.c_str();
		
		// TODO: Radio/Check support.
		bMenuHasIcon = false;
		switch ((menu->flags & GMF_ITEM_MASK))
		{
			case GMF_ITEM_SEPARATOR:
				// Separator.
				mnuItem = gtk_separator_menu_item_new();
				gtk_widget_set_sensitive(mnuItem, FALSE);
				radioGroup = NULL;
				bSetCallbackHandler = false;
				break;
			
			case GMF_ITEM_CHECK:
				// Check menu item.
				mnuItem = gtk_check_menu_item_new_with_mnemonic(sMenuText_cstr);
				radioGroup = NULL;
				bSetCallbackHandler = true;
				break;
			
			case GMF_ITEM_RADIO:
				// Radio menu item.
				mnuItem = gtk_radio_menu_item_new_with_mnemonic(radioGroup, sMenuText_cstr);
				radioGroup = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(mnuItem));
				bSetCallbackHandler = true;
				break;
				
			default:
				// Not a special menu item.
				radioGroup = NULL;
				
				// If this isn't a submenu, set the callback handler.
				bSetCallbackHandler = ((menu->flags & GMF_ITEM_MASK) != GMF_ITEM_SUBMENU);
				
				// Check if an icon was specified.
				if (menu->icon > 0)
				{
					bMenuHasIcon = true;
					mnuItem = gtk_image_menu_item_new_with_mnemonic(sMenuText_cstr);
				}
				else
				{
					mnuItem = gtk_menu_item_new_with_mnemonic(sMenuText_cstr);
				}
				
				break;
		}
		
		gtk_widget_show(mnuItem);
		gtk_container_add(GTK_CONTAINER(container), mnuItem);
		
		// Check if an icon is specified.
		if (bMenuHasIcon)
		{
			// Icon specified.
			const char* iconName = NULL;
			if (menu->icon > IDIM_MENU_ICON && menu->icon < IDIM_MENU_ICON_MAX)
			{
				// Valid icon.
				iconName = gens_gtk_menu_icons[menu->icon - IDIM_MENU_ICON].name;
			}
			
			icon = NULL;
			if (iconName)
			{
				if (gens_gtk_menu_icons[menu->icon - IDIM_MENU_ICON].stock)
				{
					// GTK+ stock icon.
					icon = gtk_image_new_from_stock(iconName, GTK_ICON_SIZE_MENU);
					if (!icon)
					{
						// Icon not found.
						LOG_MSG(gens, LOG_MSG_LEVEL_WARNING,
							"GTK+ stock icon not found: '%s'", iconName);
					}
				}
				else
				{
					// Load an icon from a file.
					icon = create_pixmap(iconName);
					if (!icon)
					{
						// Icon not found.
						LOG_MSG(gens, LOG_MSG_LEVEL_WARNING,
							"Icon file not found: '%s'", iconName);
					}
				}
			}
			
			if (icon)
			{
				// Icon loaded.
				gtk_widget_show(icon);
				gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(mnuItem), icon);
			}
		}
		
		// Check for an accelerator.
		if (menu->accelKey != 0)
		{
			// Accelerator specified.
			int accelModifier = 0;
			guint accelKey;
			
			// Determine the modifier.
			if (menu->accelModifier & GMAM_CTRL)
				accelModifier |= GDK_CONTROL_MASK;
			if (menu->accelModifier & GMAM_ALT)
				accelModifier |= GDK_MOD1_MASK;
			if (menu->accelModifier & GMAM_SHIFT)
				accelModifier |= GDK_SHIFT_MASK;
			
			// Determine the key.
			// TODO: Add more special keys.
			switch (menu->accelKey)
			{
				case GMAK_BACKSPACE:
					accelKey = GDK_BackSpace;
					break;
				
				case GMAK_ENTER:
					accelKey = GDK_Return;
					break;
				
				case GMAK_TAB:
					accelKey = GDK_Tab;
					break;
				
				case GMAK_F1: case GMAK_F2:  case GMAK_F3:  case GMAK_F4:
				case GMAK_F5: case GMAK_F6:  case GMAK_F7:  case GMAK_F8:
				case GMAK_F9: case GMAK_F10: case GMAK_F11: case GMAK_F12:
					accelKey = (menu->accelKey - GMAK_F1) + GDK_F1;
					break;
					
				default:
					accelKey = menu->accelKey;
					break;
			}
			
			// Add the accelerator.
			if (accel_group)
			{
				gtk_widget_add_accelerator(mnuItem, "activate", accel_group,
							   accelKey, (GdkModifierType)accelModifier,
							   GTK_ACCEL_VISIBLE);
			}
		}
		
		// Check for a submenu.
		if (((menu->flags & GMF_ITEM_MASK) == GMF_ITEM_SUBMENU) && (menu->submenu))
		{
			// Submenu.
			subMenu = gtk_menu_new();
			gtk_menu_item_set_submenu(GTK_MENU_ITEM(mnuItem), subMenu);
			
			// Parse the submenu.
			gens_menu_parse(menu->submenu, subMenu, accel_group);
		}
		
		if (bSetCallbackHandler)
		{
			// Set the callback handler.
			g_signal_connect((gpointer)mnuItem, "activate",
					 G_CALLBACK(gens_gtk_menu_callback), GINT_TO_POINTER(menu->id));
		}
		
		// Add the menu to the menu map. (Exception is if id is 0 or IDM_SEPARATOR.)
		if (menu->id != 0 && menu->id != IDM_SEPARATOR)
		{
			gens_menu_map.insert(gensMenuMapItem_t(menu->id, mnuItem));
		}
		
		// Next menu item.
		menu++;
	}
}


/**
 * gens_menu_find_item(): Find a menu item in the menu map.
 * @param id Menu item ID.
 * @return Menu item.
 */
GtkWidget* gens_menu_find_item(uint16_t id)
{
	// TODO: Make this a common function.
	
	gensMenuMap_t::iterator mnuIter;
	
	mnuIter = gens_menu_map.find(id);
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


/**
 * gens_gtk_menu_callback(): Menu item callback. (GTK+)
 * @param menuitem Menu item widget.
 * @param user_data Menu item ID.
 */
void gens_gtk_menu_callback(GtkMenuItem *menuitem, gpointer user_data)
{
	if (!gens_menu_do_callbacks)
		return;
	
	bool state = false;
	uint16_t menuItemID = (uint16_t)(GPOINTER_TO_INT(user_data));
	
	if (GTK_IS_RADIO_MENU_ITEM(menuitem))
	{
		// Radio menu items should only trigger the callback if they're selected.
		if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)))
			return;
		state = true;
	}
	else if (GTK_IS_CHECK_MENU_ITEM(menuitem))
	{
		// Check menu items automatically toggle, so the state value should be
		// the opposite value of its current state.
		state = !gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem));
		
		// Revert the automatic toggle if this is in the Plugins Menu.
		if ((menuItemID & 0xF000) == IDM_PLUGINS_MENU)
		{
			// Plugins menu. Revert the automatic toggle.
			gens_menu_do_callbacks = 0;
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem), state);
			gens_menu_do_callbacks = 1;
		}
	}
	
	// Run the callback function.
	gens_common_menu_callback(menuItemID, state);
}
