/***************************************************************************
 * Gens: (GTK+) Main Window.                                               *
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

#include "gens_window.hpp"
#include "gens_window_callbacks.hpp"
#include "gens_window_sync.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>

// Gens GTK+ miscellaneous functions
#include "gtk-misc.h"

#include "emulator/g_main.hpp"

#ifdef GENS_DEBUGGER
#include "debugger/debugger.hpp"
#endif /* GENS_DEBUGGER */

GtkWidget *gens_window;
GtkWidget *MenuBar;

#ifdef GENS_DEBUGGER
// Debug menu items
GtkWidget *debugMenuItems[9];
GtkWidget *debugSeparators[2];
#endif /* GENS_DEBUGGER */


GtkAccelGroup *accel_group;


// Macro to create a menu item easily.
#define NewMenuItem(MenuItemWidget, MenuItemCaption, MenuItemName, Container)			\
{												\
	MenuItemWidget = gtk_image_menu_item_new_with_mnemonic(MenuItemCaption);		\
	gtk_widget_set_name(MenuItemWidget, MenuItemName);					\
	gtk_widget_show(MenuItemWidget);							\
	gtk_container_add(GTK_CONTAINER(Container), MenuItemWidget);				\
	GLADE_HOOKUP_OBJECT(gens_window, MenuItemWidget, MenuItemName);				\
}


// Macro to create a menu item easily, with an icon.
#define NewMenuItem_Icon(MenuItemWidget, MenuItemCaption, MenuItemName, Container,		\
			 IconWidget, IconFilename)						\
{												\
	NewMenuItem(MenuItemWidget, MenuItemCaption, MenuItemName, Container);			\
	IconWidget = create_pixmap(IconFilename);						\
	gtk_widget_set_name(IconWidget, MenuItemName "_Icon");					\
	gtk_widget_show(IconWidget);								\
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(MenuItemWidget), IconWidget);		\
	GLADE_HOOKUP_OBJECT(gens_window, IconWidget, MenuItemName "_Icon");			\
}


// Macro to create a menu item easily, with a stock GTK+ icon.
#define NewMenuItem_StockIcon(MenuItemWidget, MenuItemCaption, MenuItemName, Container,		\
			      IconWidget, IconName)						\
{												\
	NewMenuItem(MenuItemWidget, MenuItemCaption, MenuItemName, Container);			\
	IconWidget = gtk_image_new_from_stock(IconName, GTK_ICON_SIZE_MENU);			\
	gtk_widget_set_name(IconWidget, MenuItemName "_Icon");					\
	gtk_widget_show(IconWidget);								\
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(MenuItemWidget), IconWidget);		\
	GLADE_HOOKUP_OBJECT(gens_window, IconWidget, MenuItemName "_Icon");			\
}


// Macro to create a menu separator.
#define NewMenuSeparator(SeparatorWidget, SeparatorName, Container)				\
{												\
	SeparatorWidget = gtk_separator_menu_item_new();					\
	gtk_widget_set_name(SeparatorWidget, SeparatorName);					\
	gtk_widget_show(SeparatorWidget);							\
	gtk_container_add(GTK_CONTAINER(Container), SeparatorWidget);				\
	gtk_widget_set_sensitive(SeparatorWidget, FALSE);					\
	GLADE_HOOKUP_OBJECT(gens_window, SeparatorWidget, SeparatorName);			\
}


// Macro to create a menu item with radio buttons.
#define NewMenuItem_Radio(MenuItemWidget, MenuItemCaption, MenuItemName, Container, State, RadioGroup)	\
{												\
	MenuItemWidget = gtk_radio_menu_item_new_with_mnemonic(RadioGroup, (MenuItemCaption));	\
	RadioGroup = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(MenuItemWidget));	\
	gtk_widget_set_name(MenuItemWidget, MenuItemName);					\
	gtk_widget_show(MenuItemWidget);							\
	gtk_container_add(GTK_CONTAINER(Container), MenuItemWidget);				\
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MenuItemWidget), State);		\
	GLADE_HOOKUP_OBJECT(gens_window, MenuItemWidget, MenuItemName);				\
}


// Macro to create a menu item with a checkbox.
#define NewMenuItem_Check(MenuItemWidget, MenuItemCaption, MenuItemName, Container, State)	\
{												\
	MenuItemWidget = gtk_check_menu_item_new_with_mnemonic(MenuItemCaption);		\
	gtk_widget_set_name(MenuItemWidget, MenuItemName);					\
	gtk_widget_show(MenuItemWidget);							\
	gtk_container_add(GTK_CONTAINER(Container), MenuItemWidget);				\
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MenuItemWidget), State);		\
	GLADE_HOOKUP_OBJECT(gens_window, MenuItemWidget, MenuItemName);				\
}


// Macro to add an accelerator to a menu item.
#define AddMenuAccelerator(MenuItemWidget, Key, Modifier)					\
{												\
	gtk_widget_add_accelerator(MenuItemWidget, "activate", accel_group,			\
				   (Key), (GdkModifierType)(Modifier),				\
				   GTK_ACCEL_VISIBLE);						\
}


// Macro to add a callback to a menu item.
#define AddMenuCallback(MenuItemWidget, CallbackFunction)					\
{												\
	g_signal_connect((gpointer)MenuItemWidget, "activate",					\
			 G_CALLBACK(CallbackFunction), NULL);					\
}


static void create_gens_window_menubar(GtkWidget *container);

#include "ui/common/gens/gens_menu.h"
static void ParseMenu(GensMenuItem_t *menu, GtkWidget *container);


static void create_gens_window_FileMenu(GtkWidget *container);
static void create_gens_window_FileMenu_ChangeState_SubMenu(GtkWidget *container);
static void create_gens_window_GraphicsMenu(GtkWidget *container);
#ifdef GENS_OPENGL
static void create_gens_window_GraphicsMenu_OpenGLRes_SubMenu(GtkWidget *container);
#endif
static void create_gens_window_GraphicsMenu_bpp_SubMenu(GtkWidget *container);
static void create_gens_window_GraphicsMenu_FrameSkip_SubMenu(GtkWidget *container);
static void create_gens_window_CPUMenu(GtkWidget *container);
#ifdef GENS_DEBUGGER
static void create_gens_window_CPUMenu_Debug_SubMenu(GtkWidget *container);
#endif /* GENS_DEBUGGER */
static void create_gens_window_CPUMenu_Country_SubMenu(GtkWidget *container);
static void create_gens_window_SoundMenu(GtkWidget *container);
static void create_gens_window_SoundMenu_Rate_SubMenu(GtkWidget *container);
static void create_gens_window_OptionsMenu(GtkWidget *container);
static void create_gens_window_OptionsMenu_SegaCDSRAMSize_SubMenu(GtkWidget *container);
static void create_gens_window_HelpMenu(GtkWidget *container);


// Set to 0 to temporarily disable callbacks.
int do_callbacks = 1;


GtkWidget* create_gens_window(void)
{
	GdkPixbuf *gens_window_icon_pixbuf;
	GtkWidget *vbox1;
	GtkWidget *handlebox1;
	GtkWidget *sdlsock;
	
	accel_group = gtk_accel_group_new();
	
	// Create the Gens window.
	CREATE_GTK_WINDOW(gens_window, "gens_window", "Gens",
			  gens_window_icon_pixbuf, "Gens2.ico");
	gtk_window_set_resizable(GTK_WINDOW(gens_window), FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(gens_window), 0);
	
	// Layout objects.
	vbox1 = gtk_vbox_new(FALSE, 0);
	gtk_widget_set_name(vbox1, "vbox1");
	gtk_widget_show(vbox1);
	gtk_container_add(GTK_CONTAINER(gens_window), vbox1);
	GLADE_HOOKUP_OBJECT(gens_window, vbox1, "vbox1");
	
	handlebox1 = gtk_handle_box_new();
	gtk_widget_set_name(handlebox1, "handlebox1");
	gtk_widget_show(handlebox1);
	gtk_box_pack_start(GTK_BOX(vbox1), handlebox1, FALSE, FALSE, 0);
	gtk_handle_box_set_shadow_type(GTK_HANDLE_BOX(handlebox1), GTK_SHADOW_NONE);
	gtk_handle_box_set_snap_edge(GTK_HANDLE_BOX(handlebox1), GTK_POS_LEFT);
	GLADE_HOOKUP_OBJECT(gens_window, vbox1, "handlebox1");
	
	// Create the menu bar.
	create_gens_window_menubar(handlebox1);
	
	// Create the SDL socket.
	sdlsock = gtk_event_box_new();
	gtk_widget_set_name(sdlsock, "sdlsock");
	gtk_box_pack_end(GTK_BOX(lookup_widget(gens_window, "vbox1")), sdlsock, 0, 0, 0);
	GLADE_HOOKUP_OBJECT(gens_window, sdlsock, "sdlsock");
	
	// Add the accel group.
	gtk_window_add_accel_group(GTK_WINDOW(gens_window), accel_group);
	
	// Callbacks for if the window is closed.
	g_signal_connect((gpointer)gens_window, "delete-event",
			 G_CALLBACK(on_gens_window_close), NULL);
	g_signal_connect((gpointer)gens_window, "destroy-event",
			 G_CALLBACK(on_gens_window_close), NULL);
	
	// Enable drag & drop for ROM loading.
	const GtkTargetEntry target_list[] =
	{
		{"text/plain", 0, 0},
		{"text/uri-list", 0, 1},
	};
	
	gtk_drag_dest_set
	(
		sdlsock,
		(GtkDestDefaults)(GTK_DEST_DEFAULT_MOTION | GTK_DEST_DEFAULT_HIGHLIGHT),
		target_list,
		G_N_ELEMENTS(target_list),
		(GdkDragAction)(GDK_ACTION_COPY | GDK_ACTION_MOVE | GDK_ACTION_LINK | GDK_ACTION_PRIVATE | GDK_ACTION_ASK)
	);
	
	// Set drag & drop callbacks.
	g_signal_connect(sdlsock, "drag-data-received",
			 G_CALLBACK(gens_window_drag_data_received), NULL);
	g_signal_connect(sdlsock, "drag-drop",
			 G_CALLBACK(gens_window_drag_drop), NULL);
	
	return gens_window;
}


/**
 * create_gens_window_menubar(): Create the menu bar.
 * @param container Container for the menu bar.
 */
static void create_gens_window_menubar(GtkWidget *container)
{
	MenuBar = gtk_menu_bar_new();
	gtk_widget_set_name(MenuBar, "MenuBar");
	gtk_widget_show(MenuBar);
	gtk_container_add(GTK_CONTAINER(container), MenuBar);
	
	// Menus
	ParseMenu(&gmiMain[0], MenuBar);
#if 0
	create_gens_window_FileMenu(MenuBar);
	create_gens_window_GraphicsMenu(MenuBar);
	create_gens_window_CPUMenu(MenuBar);
	create_gens_window_SoundMenu(MenuBar);
	create_gens_window_OptionsMenu(MenuBar);
	create_gens_window_HelpMenu(MenuBar);
#endif
}


/**
 * ParseMenu(): Parse the menu structs.
 * @param menu First item of the array of menu structs to parse.
 * @param container Container to add the menu items to.
 */
static void ParseMenu(GensMenuItem_t *menu, GtkWidget *container)
{
	GtkWidget *mnuItem, *subMenu;
	GtkWidget *icon;
	char widgetName[64];
	char *mnuText, *mnemonicPos;
	
	while (menu->id != 0)
	{
		if ((menu->flags & GMF_ITEM_MASK) == GMF_ITEM_SEPARATOR)
		{
			// Separator.
			mnuItem = gtk_separator_menu_item_new();
			
			sprintf(widgetName, "mnu_sep_0x%08X_0x%04X", (unsigned int)menu, menu->id);
			gtk_widget_set_name(mnuItem, widgetName);
			
			gtk_widget_set_sensitive(mnuItem, FALSE);
			gtk_widget_show(mnuItem);
			gtk_container_add(GTK_CONTAINER(container), mnuItem);
			
			g_object_set_data_full(G_OBJECT(gens_window), widgetName,
					       g_object_ref(mnuItem),
					       (GDestroyNotify)g_object_unref);
			
			// Next menu item.
			menu++;
			continue;
		}
		
		// Convert the Win32/Qt mnemonic symbol ("&") to the GTK+ mnemonic symbol ("_").
		mnuText = strdup(menu->text);
		mnemonicPos = strchr(mnuText, '&');
		if (mnemonicPos)
			*mnemonicPos = '_';
		
		// TODO: Radio/Check support.
		if (menu->flags & GMF_ICON_MASK)
			mnuItem = gtk_image_menu_item_new_with_mnemonic(mnuText);
		else
			mnuItem = gtk_menu_item_new_with_mnemonic(mnuText);
		free(mnuText);
		
		sprintf(widgetName, "mnu_0x%08X_0x%04X", (unsigned int)menu, menu->id);
		gtk_widget_set_name(mnuItem, widgetName);
		gtk_widget_show(mnuItem);
		gtk_container_add(GTK_CONTAINER(container), mnuItem);
		
		g_object_set_data_full(G_OBJECT(gens_window), widgetName,
				       g_object_ref(mnuItem),
				       (GDestroyNotify)g_object_unref);
		
		// Check if an icon is specified.
		if (menu->flags & GMF_ICON_MASK)
		{
			// Icon specified.
			if (!menu->iconName)
				break;
			
			icon = NULL;
			switch (menu->flags & GMF_ICON_MASK)
			{
				case GMF_ICON_STOCK:
					// GTK+ stock icon.
					icon = gtk_image_new_from_stock(menu->iconName, GTK_ICON_SIZE_MENU);
					if (!icon)
					{
						// Icon not found.
						fprintf(stderr, "%s: GTK+ stock icon not found: %s\n", __func__, menu->iconName);
					}
					break;
				
				case GMF_ICON_FILE:
					// Load an icon from a file.
					icon = create_pixmap(menu->iconName);
					if (!icon)
					{
						// Icon not found.
						fprintf(stderr, "%s: Icon file not found: %s\n", __func__, menu->iconName);
					}
					break;
				
				default:
					// Unknown icon type.
					fprintf(stderr, "%s: Unknown icon type: 0x%04X\n", __func__, (menu->flags & GMF_ICON_MASK));
					break;
			}
			
			if (icon)
			{
				// Icon loaded.
				sprintf(widgetName, "mnu_icon_0x%08X_0x%04X", (unsigned int)menu, menu->id);
				gtk_widget_set_name(icon, widgetName);
				gtk_widget_show(icon);
				gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(mnuItem), icon);
				
				g_object_set_data_full(G_OBJECT(gens_window), widgetName,
						       g_object_ref(icon),
						       (GDestroyNotify)g_object_unref);
			}
		}
		
		// Check for a submenu.
		if (((menu->flags & GMF_ITEM_MASK) == GMF_ITEM_SUBMENU) && (menu->submenu))
		{
			// Submenu.
			subMenu = gtk_menu_new();
			
			sprintf(widgetName, "mnu_sub_0x%08X_0x%04X", (unsigned int)menu, menu->id);
			gtk_widget_set_name(subMenu, widgetName);
			gtk_menu_item_set_submenu(GTK_MENU_ITEM(mnuItem), subMenu);
			
			g_object_set_data_full(G_OBJECT(gens_window), widgetName,
					       g_object_ref(subMenu),
					       (GDestroyNotify)g_object_unref);
			
			// Parse the submenu.
			ParseMenu(menu->submenu, subMenu);
		}
		
		// Next menu item.
		menu++;
	}
}


/**
 * create_gens_window_FileMenu(): Create the File menu.
 * @param container Container for this menu.
 */
static void create_gens_window_FileMenu(GtkWidget *container)
{
	GtkWidget *File;
	GtkWidget *FileMenu;
	GtkWidget *FileMenu_OpenROM;		GtkWidget *FileMenu_OpenROM_Icon;
#ifdef GENS_CDROM
	GtkWidget *FileMenu_BootCD;		GtkWidget *FileMenu_BootCD_Icon;
#endif
	GtkWidget *FileMenu_Netplay;		GtkWidget *FileMenu_Netplay_Icon;
	GtkWidget *FileMenu_ROMHistory;		GtkWidget *FileMenu_ROMHistory_Icon;
	GtkWidget *FileMenu_CloseROM;		GtkWidget *FileMenu_CloseROM_Icon;
	GtkWidget *FileMenu_Separator1;
	GtkWidget *FileMenu_GameGenie;		GtkWidget *FileMenu_GameGenie_Icon;
	GtkWidget *FileMenu_Separator2;
	GtkWidget *FileMenu_LoadState;		GtkWidget *FileMenu_LoadState_Icon;
	GtkWidget *FileMenu_SaveState;		GtkWidget *FileMenu_SaveState_Icon;
	GtkWidget *FileMenu_QuickLoad;		GtkWidget *FileMenu_QuickLoad_Icon;
	GtkWidget *FileMenu_QuickSave;		GtkWidget *FileMenu_QuickSave_Icon;
	GtkWidget *FileMenu_ChangeState;	GtkWidget *FileMenu_ChangeState_Icon;
	GtkWidget *FileMenu_Separator3;
	GtkWidget *FileMenu_Quit;		GtkWidget *FileMenu_Quit_Icon;
	
	// File
	NewMenuItem(File, "_File", "File", container);
	
	// Menu object for the FileMenu
	FileMenu = gtk_menu_new();
	gtk_widget_set_name(FileMenu, "FileMenu");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(File), FileMenu);
	
	// Open ROM...
	NewMenuItem_StockIcon(FileMenu_OpenROM, "_Open ROM...", "FileMenu_OpenROM", FileMenu,
			      FileMenu_OpenROM_Icon, "gtk-open");
	AddMenuAccelerator(FileMenu_OpenROM, GDK_O, GDK_CONTROL_MASK);
	AddMenuCallback(FileMenu_OpenROM, on_FileMenu_OpenROM_activate);
	
#ifdef GENS_CDROM
	// Boot CD
	NewMenuItem_StockIcon(FileMenu_BootCD, "_Boot CD", "FileMenu_BootCD", FileMenu,
			      FileMenu_BootCD_Icon, "gtk-cdrom");
	AddMenuAccelerator(FileMenu_BootCD, GDK_B, GDK_CONTROL_MASK);
	AddMenuCallback(FileMenu_BootCD, on_FileMenu_BootCD_activate);
#endif
	
	// Netplay (currently disabled)
	NewMenuItem_Icon(FileMenu_Netplay, "_Netplay", "FileMenu_Netplay", FileMenu,
			 FileMenu_Netplay_Icon, "modem.png");
	gtk_widget_set_sensitive(FileMenu_Netplay, FALSE);
	
	// ROM History
	NewMenuItem_Icon(FileMenu_ROMHistory, "ROM _History", "FileMenu_ROMHistory", FileMenu,
			 FileMenu_ROMHistory_Icon, "history.png");
	// ROM History submenu is handled by Sync_Gens_Window_FileMenu().
	
	// Close ROM
	NewMenuItem_StockIcon(FileMenu_CloseROM, "_Close ROM", "FileMenu_CloseROM", FileMenu,
			      FileMenu_CloseROM_Icon, "gtk-close");
	gtk_widget_add_accelerator(FileMenu_CloseROM, "activate", accel_group,
				   GDK_W, (GdkModifierType)GDK_CONTROL_MASK,
				   GTK_ACCEL_VISIBLE);
	AddMenuCallback(FileMenu_CloseROM, on_FileMenu_CloseROM_activate);
	
	// Separator
	NewMenuSeparator(FileMenu_Separator1, "FileMenu_Separator1", FileMenu);
	
	// Game Genie
	NewMenuItem_Icon(FileMenu_GameGenie, "_Game Genie", "FileMenu_GameGenie", FileMenu,
			 FileMenu_GameGenie_Icon, "password.png");
	gtk_widget_add_accelerator(FileMenu_GameGenie, "activate", accel_group,
				   GDK_G, (GdkModifierType)GDK_CONTROL_MASK,
				   GTK_ACCEL_VISIBLE);
	AddMenuCallback(FileMenu_GameGenie, on_FileMenu_GameGenie_activate);
	
	// Separator
	NewMenuSeparator(FileMenu_Separator2, "FileMenu_Separator2", FileMenu);
	
	// Load State...
	NewMenuItem_StockIcon(FileMenu_LoadState, "_Load State...", "FileMenu_LoadState", FileMenu,
			      FileMenu_LoadState_Icon, "gtk-open");
	AddMenuAccelerator(FileMenu_LoadState, GDK_F8, GDK_SHIFT_MASK);
	AddMenuCallback(FileMenu_LoadState, on_FileMenu_LoadState_activate);
	
	// Save State As...
	NewMenuItem_StockIcon(FileMenu_SaveState, "_Save State As...", "FileMenu_SaveState", FileMenu,
			      FileMenu_SaveState_Icon, "gtk-save-as");
	AddMenuAccelerator(FileMenu_SaveState, GDK_F5, GDK_SHIFT_MASK);
	AddMenuCallback(FileMenu_SaveState, on_FileMenu_SaveState_activate);
	
	// Quick Load
	NewMenuItem_StockIcon(FileMenu_QuickLoad, "Quick Load", "FileMenu_QuickLoad", FileMenu,
			      FileMenu_QuickLoad_Icon, "gtk-refresh");
	AddMenuAccelerator(FileMenu_QuickLoad, GDK_F8, 0);
	AddMenuCallback(FileMenu_QuickLoad, on_FileMenu_QuickLoad_activate);
	
	// Quick Save
	NewMenuItem_StockIcon(FileMenu_QuickSave, "Quick Save", "FileMenu_QuickSave", FileMenu,
			      FileMenu_QuickSave_Icon, "gtk-save");
	AddMenuAccelerator(FileMenu_QuickSave, GDK_F5, 0);
	AddMenuCallback(FileMenu_QuickSave, on_FileMenu_QuickSave_activate);
	
	// Change State
	NewMenuItem_StockIcon(FileMenu_ChangeState, "Change State", "FileMenu_ChangeState", FileMenu,
			      FileMenu_ChangeState_Icon, "gtk-revert-to-saved");
	// Change State submenu
	create_gens_window_FileMenu_ChangeState_SubMenu(FileMenu_ChangeState);
	
	// Separator
	NewMenuSeparator(FileMenu_Separator3, "FileMenu_Separator3", FileMenu);
	
	// Quit
	NewMenuItem_StockIcon(FileMenu_Quit, "_Quit", "FileMenu_Quit", FileMenu,
			      FileMenu_Quit_Icon, "gtk-quit");
	AddMenuAccelerator(FileMenu_Quit, GDK_Q, GDK_CONTROL_MASK);
	AddMenuCallback(FileMenu_Quit, on_FileMenu_Quit_activate);
}


/**
 * gens_window_FileMenu_ChangeState_SubMenu(): Create the File, Change State submenu.
 * @param container Container for this menu.
 */
static void create_gens_window_FileMenu_ChangeState_SubMenu(GtkWidget *container)
{
	GtkWidget *SubMenu;
	GtkWidget *SlotItem;
	GSList *SlotGroup = NULL;
	
	int i;
	char ObjName[64];
	char SlotName[8];
	
	// Create the submenu.
	SubMenu = gtk_menu_new();
	gtk_widget_set_name(SubMenu, "FileMenu_ChangeState_SubMenu");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(container), SubMenu);
	
	// Create the save slot entries.
	for (i = 0; i < 10; i++)
	{
		sprintf(SlotName, "%d", i);
		sprintf(ObjName, "FileMenu_ChangeState_SubMenu_%s", SlotName);
		NewMenuItem_Radio(SlotItem, SlotName, ObjName, SubMenu, (i == 0 ? TRUE : FALSE), SlotGroup);
		g_signal_connect((gpointer)SlotItem, "activate",
				 G_CALLBACK(on_FileMenu_ChangeState_SubMenu_SlotItem_activate),
				 GINT_TO_POINTER(i));
	}
}


/**
 * create_gens_window_GraphicsMenu(): Create the Graphics menu.
 * @param container Container for this menu.
 */
static void create_gens_window_GraphicsMenu(GtkWidget *container)
{
	GtkWidget *Graphics;
	GtkWidget *GraphicsMenu;
	GtkWidget *GraphicsMenu_FullScreen;	GtkWidget *GraphicsMenu_FullScreen_Icon;
	GtkWidget *GraphicsMenu_VSync;
	GtkWidget *GraphicsMenu_Stretch;
	GtkWidget *GraphicsMenu_Separator1;
#ifdef GENS_OPENGL
	GtkWidget *GraphicsMenu_OpenGL;
	GtkWidget *GraphicsMenu_OpenGLFilter;
	GtkWidget *GraphicsMenu_OpenGLRes;
#endif
	GtkWidget *GraphicsMenu_bpp;
	GtkWidget *GraphicsMenu_Separator2;
	GtkWidget *GraphicsMenu_ColorAdjust;	GtkWidget *GraphicsMenu_ColorAdjust_Icon;
	GtkWidget *GraphicsMenu_Render;		GtkWidget *GraphicsMenu_Render_Icon;
	GtkWidget *GraphicsMenu_Separator3;
	GtkWidget *GraphicsMenu_SpriteLimit;
	GtkWidget *GraphicsMenu_Separator4;
	GtkWidget *GraphicsMenu_FrameSkip;	GtkWidget *GraphicsMenu_FrameSkip_Icon;
	GtkWidget *GraphicsMenu_Separator5;
	GtkWidget *GraphicsMenu_ScreenShot;	GtkWidget *GraphicsMenu_ScreenShot_Icon;
	
	// Graphics
	NewMenuItem(Graphics, "_Graphics", "Graphics", container);
	
	// Menu object for the GraphicsMenu
	GraphicsMenu = gtk_menu_new();
	gtk_widget_set_name(GraphicsMenu, "GraphicsMenu");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(Graphics), GraphicsMenu);
	
	// Full Screen
	NewMenuItem_StockIcon(GraphicsMenu_FullScreen, "_Full Screen", "GraphicsMenu_FullScreen", GraphicsMenu,
			      GraphicsMenu_FullScreen_Icon, "gtk-fullscreen");
	AddMenuAccelerator(GraphicsMenu_FullScreen, GDK_Return, GDK_MOD1_MASK);
	AddMenuCallback(GraphicsMenu_FullScreen, on_GraphicsMenu_FullScreen_activate);
	
	// VSync
	NewMenuItem_Check(GraphicsMenu_VSync, "_VSync", "GraphicsMenu_VSync", GraphicsMenu, FALSE);
	AddMenuAccelerator(GraphicsMenu_VSync, GDK_F3, GDK_SHIFT_MASK);
	AddMenuCallback(GraphicsMenu_VSync, on_GraphicsMenu_VSync_activate);
	
	// Stretch
	NewMenuItem_Check(GraphicsMenu_Stretch, "_Stretch", "GraphicsMenu_Stretch", GraphicsMenu, FALSE);
	AddMenuAccelerator(GraphicsMenu_Stretch, GDK_F2, GDK_SHIFT_MASK);
	AddMenuCallback(GraphicsMenu_Stretch, on_GraphicsMenu_Stretch_activate);
	
	// Separator
	NewMenuSeparator(GraphicsMenu_Separator1, "GraphicsMenu_Separator1", GraphicsMenu);
	
#ifdef GENS_OPENGL
	// OpenGL
	NewMenuItem_Check(GraphicsMenu_OpenGL, "Open_GL", "GraphicsMenu_OpenGL", GraphicsMenu, FALSE);
	AddMenuAccelerator(GraphicsMenu_OpenGL, GDK_r, GDK_SHIFT_MASK);
	AddMenuCallback(GraphicsMenu_OpenGL, on_GraphicsMenu_OpenGL_activate);
	
	// OpenGL Linear Filter
	NewMenuItem_Check(GraphicsMenu_OpenGLFilter, "OpenGL _Linear Filter", "GraphicsMenu_OpenGLFilter", GraphicsMenu, FALSE);
	AddMenuCallback(GraphicsMenu_OpenGLFilter, on_GraphicsMenu_OpenGLFilter_activate);
	
	// OpenGL Resolution
	NewMenuItem(GraphicsMenu_OpenGLRes, "OpenGL Resolution", "GraphicsMenu_OpenGLRes", GraphicsMenu);
	// OpenGL Resolution submenu
	create_gens_window_GraphicsMenu_OpenGLRes_SubMenu(GraphicsMenu_OpenGLRes);
#endif
	
	// Bits per pixel (OpenGL mode) [TODO: Where is this value actually used?]
	NewMenuItem(GraphicsMenu_bpp, "Bits per pixel", "GraphicsMenu_bpp", GraphicsMenu);
	// Bits per pixel submenu
	create_gens_window_GraphicsMenu_bpp_SubMenu(GraphicsMenu_bpp);
	
	// Separator
	NewMenuSeparator(GraphicsMenu_Separator2, "GraphicsMenu_Separator2", GraphicsMenu);
	
	// Color Adjust
	NewMenuItem_StockIcon(GraphicsMenu_ColorAdjust, "Color Adjust...", "GraphicsMenu_ColorAdjust", GraphicsMenu,
			      GraphicsMenu_ColorAdjust_Icon, "gtk-select-color");
	AddMenuCallback(GraphicsMenu_ColorAdjust, on_GraphicsMenu_ColorAdjust_activate);
	
	// Render
	NewMenuItem_Icon(GraphicsMenu_Render, "_Render", "GraphicsMenu_Render", GraphicsMenu,
			 GraphicsMenu_Render_Icon, "viewmag.png");
	// Render submenu
	Sync_Gens_Window_GraphicsMenu_Render(GraphicsMenu_Render);
	
	// Separator
	NewMenuSeparator(GraphicsMenu_Separator3, "GraphicsMenu_Separator3", GraphicsMenu);
	
	// Sprite Limit
	NewMenuItem_Check(GraphicsMenu_SpriteLimit, "Sprite Limit", "GraphicsMenu_SpriteLimit", GraphicsMenu, TRUE);
	AddMenuCallback(GraphicsMenu_SpriteLimit, on_GraphicsMenu_SpriteLimit_activate);
	
	// Separator
	NewMenuSeparator(GraphicsMenu_Separator4, "GraphicsMenu_Separator4", GraphicsMenu);
	
	// Frame Skip
	NewMenuItem_Icon(GraphicsMenu_FrameSkip, "Frame Skip", "GraphicsMenu_FrameSkip", GraphicsMenu,
			 GraphicsMenu_FrameSkip_Icon, "2rightarrow.png");
	// Frame Skip submenu
	create_gens_window_GraphicsMenu_FrameSkip_SubMenu(GraphicsMenu_FrameSkip);
	
	// Separator
	NewMenuSeparator(GraphicsMenu_Separator5, "GraphicsMenu_Separator5", GraphicsMenu);
	
	// Screen Shot
	NewMenuItem_StockIcon(GraphicsMenu_ScreenShot, "Screen Shot", "GraphicsMenu_ScreenShot", GraphicsMenu,
			      GraphicsMenu_ScreenShot_Icon, "gtk-copy");
	AddMenuAccelerator(GraphicsMenu_ScreenShot, GDK_BackSpace, GDK_SHIFT_MASK);
	AddMenuCallback(GraphicsMenu_ScreenShot, on_GraphicsMenu_ScreenShot_activate);
}


#ifdef GENS_OPENGL
/**
 * create_gens_window_GraphicsMenu_OpenGLRes_SubMenu(): Create the Graphics, OpenGL Resolution submenu.
 * @param container Container for this menu.
 */
static void create_gens_window_GraphicsMenu_OpenGLRes_SubMenu(GtkWidget *container)
{
	GtkWidget *SubMenu;
	GtkWidget *ResItem;
	GSList *ResGroup = NULL;
	int resValue;	// 0xWWWWHHHH
	
	// TODO: Move this array somewhere else.
	int resolutions[5][2] =
	{
		{320, 240},
		{640, 480},
		{800, 600},
		{1024, 768},
		{-1, -1}, // Custom
	};
	
	int i;
	char ObjName[64];
	char ResName[16];
	
	// Create the submenu.
	SubMenu = gtk_menu_new();
	gtk_widget_set_name(SubMenu, "GraphicsMenu_OpenGLRes_SubMenu");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(container), SubMenu);
	
	// Create the resolution entries.
	for (i = 0; i < 5; i++)
	{
		if (resolutions[i][0] > 0)
			sprintf(ResName, "%dx%d", resolutions[i][0], resolutions[i][1]);
		else
			strcpy(ResName, "Custom");
		
		sprintf(ObjName, "GraphicsMenu_OpenGLRes_SubMenu_%s", ResName);
		NewMenuItem_Radio(ResItem, ResName, ObjName, SubMenu, (i == 0 ? TRUE : FALSE), ResGroup);
		resValue = (resolutions[i][0] == -1 ? 0 : (resolutions[i][0] << 16 | resolutions[i][1]));
		g_signal_connect((gpointer)ResItem, "activate",
				 G_CALLBACK(on_GraphicsMenu_OpenGLRes_SubMenu_ResItem_activate),
				 GINT_TO_POINTER(resValue));
	}
}
#endif


/**
 * create_gens_window_GraphicsMenu_bpp_SubMenu(): Create the Graphics, Bits per pixel submenu.
 * @param container Container for this menu.
 */
static void create_gens_window_GraphicsMenu_bpp_SubMenu(GtkWidget *container)
{
	GtkWidget *SubMenu;
	GtkWidget *bppItem;
	GSList *bppGroup = NULL;
	
	// TODO: Move this array somewhere else.
	int bpp[3] = {15, 16, 32};
	const char* bppStr[3] = {"15 (555)", "16 (565)", "32"};
	int i;
	char ObjName[64];
	
	// Create the submenu.
	SubMenu = gtk_menu_new();
	gtk_widget_set_name(SubMenu, "GraphicsMenu_bpp_SubMenu");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(container), SubMenu);
	
	// Create the bits per pixel entries.
	for (i = 0; i < 3; i++)
	{
		sprintf(ObjName, "GraphicsMenu_bpp_SubMenu_%d", bpp[i]);
		NewMenuItem_Radio(bppItem, bppStr[i], ObjName, SubMenu, (i == 1 ? TRUE : FALSE), bppGroup);
		g_signal_connect((gpointer)bppItem, "activate",
				 G_CALLBACK(on_GraphicsMenu_bpp_SubMenu_bppItem_activate),
				 GINT_TO_POINTER(bpp[i]));
	}
}


/**
 * create_gens_window_GraphicsMenu_FrameSkip_SubMenu(): Create the Graphics, Frame Skip submenu.
 * @param container Container for this menu.
 */
static void create_gens_window_GraphicsMenu_FrameSkip_SubMenu(GtkWidget *container)
{
	GtkWidget *SubMenu;
	GtkWidget *FSItem;
	GSList *FSGroup = NULL;
	
	int i;
	char FSName[8];
	char ObjName[64];
	
	// Create the submenu.
	SubMenu = gtk_menu_new();
	gtk_widget_set_name(SubMenu, "GraphicsMenu_FrameSkip_SubMenu");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(container), SubMenu);
	
	// Create the frame skip entries.
	for (i = -1; i <= 8; i++)
	{
		if (i >= 0)
			sprintf(FSName, "%d", i);
		else
			strcpy(FSName, "Auto");
		sprintf(ObjName, "GraphicsMenu_FrameSkip_SubMenu_%s", FSName);
		NewMenuItem_Radio(FSItem, FSName, ObjName, SubMenu, (i == -1 ? TRUE : FALSE), FSGroup);
		g_signal_connect((gpointer)FSItem, "activate",
				 G_CALLBACK(on_GraphicsMenu_FrameSkip_SubMenu_FSItem_activate),
				 GINT_TO_POINTER(i));
	}
}


/**
 * create_gens_window_CPUMenu(): Create the CPU menu.
 * @param container Container for this menu.
 */
static void create_gens_window_CPUMenu(GtkWidget *container)
{
	GtkWidget *CPU;
	GtkWidget *CPUMenu;
#ifdef GENS_DEBUGGER
	GtkWidget *CPUMenu_Debug;
	GtkWidget *CPUMenu_Separator1;
#endif /* GENS_DEBUGGER */
	GtkWidget *CPUMenu_Country;
	GtkWidget *CPUMenu_Separator2;
	GtkWidget *CPUMenu_HardReset;		GtkWidget *CPUMenu_HardReset_Icon;
	GtkWidget *CPUMenu_Reset68000;
	GtkWidget *CPUMenu_ResetMain68000;
	GtkWidget *CPUMenu_ResetSub68000;
	GtkWidget *CPUMenu_ResetMainSH2;
	GtkWidget *CPUMenu_ResetSubSH2;
	GtkWidget *CPUMenu_ResetZ80;
	GtkWidget *CPUMenu_Separator3;
	GtkWidget *CPUMenu_SegaCD_PerfectSync;
	
	// CPU
	NewMenuItem(CPU, "_CPU", "CPU", container);
	
	// Menu object for the CPUMenu
	CPUMenu = gtk_menu_new();
	gtk_widget_set_name(CPUMenu, "CPUMenu");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(CPU), CPUMenu);
	
#ifdef GENS_DEBUGGER
	// Debug
	NewMenuItem(CPUMenu_Debug, "_Debug", "CPUMenu_Debug", CPUMenu);
	// Debug submenu
	create_gens_window_CPUMenu_Debug_SubMenu(CPUMenu_Debug);
	
	// Separator
	NewMenuSeparator(CPUMenu_Separator1, "CPUMenu_Separator1", CPUMenu);
#endif /* GENS_DEBUGGER */
	
	// Country
	NewMenuItem(CPUMenu_Country, "_Country", "CPUMenu_Country", CPUMenu);
	// Country submenu
	create_gens_window_CPUMenu_Country_SubMenu(CPUMenu_Country);
	
	// Separator
	NewMenuSeparator(CPUMenu_Separator2, "CPUMenu_Separator2", CPUMenu);
	
	// Hard Reset
	NewMenuItem_StockIcon(CPUMenu_HardReset, "Hard _Reset", "CPUMenu_HardReset", CPUMenu,
			      CPUMenu_HardReset_Icon, "gtk-refresh");
	// Tab doesn't seem to work here...
	// Leaving this line in anyway so the accelerator is displayed in the menu.
	AddMenuAccelerator(CPUMenu_HardReset, GDK_Tab, 0);
	AddMenuCallback(CPUMenu_HardReset, on_CPUMenu_HardReset_activate);
	
	// Reset 68000 (same as Reset Main 68000, but shows up if the Sega CD isn't enabled.)
	NewMenuItem(CPUMenu_Reset68000, "Reset 68000", "CPUMenu_Reset68000", CPUMenu);
	AddMenuCallback(CPUMenu_Reset68000, on_CPUMenu_ResetMain68000_activate);
	
	// Reset Main 68000
	NewMenuItem(CPUMenu_ResetMain68000, "Reset Main 68000", "CPUMenu_ResetMain68000", CPUMenu);
	AddMenuCallback(CPUMenu_ResetMain68000, on_CPUMenu_ResetMain68000_activate);

	// Reset Sub 68000
	NewMenuItem(CPUMenu_ResetSub68000, "Reset Sub 68000", "CPUMenu_ResetSub68000", CPUMenu);
	AddMenuCallback(CPUMenu_ResetSub68000, on_CPUMenu_ResetSub68000_activate);
	
	// Reset Main SH2
	NewMenuItem(CPUMenu_ResetMainSH2, "Reset Main SH2", "CPUMenu_ResetMainSH2", CPUMenu);
	AddMenuCallback(CPUMenu_ResetMainSH2, on_CPUMenu_ResetMainSH2_activate);
	
	// Reset Sub SH2
	NewMenuItem(CPUMenu_ResetSubSH2, "Reset Sub SH2", "CPUMenu_ResetSubSH2", CPUMenu);
	AddMenuCallback(CPUMenu_ResetSubSH2, on_CPUMenu_ResetSubSH2_activate);
	
	// Reset Z80
	NewMenuItem(CPUMenu_ResetZ80, "Reset Z80", "CPUMenu_ResetZ80", CPUMenu);
	AddMenuCallback(CPUMenu_ResetZ80, on_CPUMenu_ResetZ80_activate);
	
	// Separator
	NewMenuSeparator(CPUMenu_Separator3, "CPUMenu_Separator3", CPUMenu);
	
	// SegaCD Perfect Sync
	NewMenuItem_Check(CPUMenu_SegaCD_PerfectSync, "SegaCD Perfect Sync (SLOW)", "CPUMenu_SegaCD_PerfectSync", CPUMenu, FALSE);
	AddMenuCallback(CPUMenu_SegaCD_PerfectSync, on_CPUMenu_SegaCD_PerfectSync_activate);
}


#ifdef GENS_DEBUGGER
/**
 * create_gens_window_CPUMenu_Debug_SubMenu(): Create the CPU, Debug submenu.
 * @param container Container for this menu.
 */
static void create_gens_window_CPUMenu_Debug_SubMenu(GtkWidget *container)
{
	GtkWidget *SubMenu;
	
	// TODO: Move this array somewhere else.
	const char* DebugStr[9] =
	{
		"_Genesis - 68000",
		"Genesis - _Z80",
		"Genesis - _VDP",
		"_SegaCD - 68000",
		"SegaCD - _CDC",
		"SegaCD - GF_X",
		"32X - Main SH2",
		"32X - Sub SH2",
		"32X - VDP",
	};
	
	int i;
	char ObjName[64];
	
	// Create the submenu.
	SubMenu = gtk_menu_new();
	gtk_widget_set_name(SubMenu, "CPUMenu_Debug_SubMenu");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(container), SubMenu);
	
	// Create the debug entries.
	for (i = 0; i < 9; i++)
	{
		sprintf(ObjName, "CPUMenu_Debug_SubMenu_%d", i + 1);
		NewMenuItem_Check(debugMenuItems[i], DebugStr[i], ObjName, SubMenu, FALSE);
		g_signal_connect((gpointer)debugMenuItems[i], "activate",
				 G_CALLBACK(on_CPUMenu_Debug_SubMenu_activate),
				 GINT_TO_POINTER(i + 1));
		if (i % 3 == 2 && i < 6)
		{
			// Every three entires, add a separator.
			sprintf(ObjName, "CPUMenu_Debug_SubMenu_Sep%d", (i / 3) + 1);
			NewMenuSeparator(debugSeparators[i / 3], ObjName, SubMenu);
		}
	}
}
#endif /* GENS_DEBUGGER */


/**
 * create_gens_window_CPUMenu_Country_SubMenu(): Create the CPU, Country submenu.
 * @param container Container for this menu.
 */
static void create_gens_window_CPUMenu_Country_SubMenu(GtkWidget *container)
{
	GtkWidget *SubMenu;
	GtkWidget *CountryItem;
	GSList *CountryGroup = NULL;
	GtkWidget *CPUMenu_Country_SubMenu_Separator;
	GtkWidget *CPUMenu_Country_SubMenu_AutoDetectOrder;
	
	// TODO: Move this array somewhere else.
	const char* CountryCodes[5] =
	{
		"Auto Detect",
		"Japan (NTSC)",
		"USA (NTSC)",
		"Europe (PAL)",
		"Japan (PAL)",
	};
	
	int i;
	char ObjName[64];
	
	// Create the submenu.
	SubMenu = gtk_menu_new();
	gtk_widget_set_name(SubMenu, "CPUMenu_Country_SubMenu");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(container), SubMenu);
	
	// Create the country code entries.
	for (i = 0; i < 5; i++)
	{
		if (i == 0)
			strcpy(ObjName, "CPUMenu_Country_SubMenu_Auto");
		else
			sprintf(ObjName, "CPUMenu_Country_SubMenu_%d", i - 1);
		NewMenuItem_Radio(CountryItem, CountryCodes[i], ObjName, SubMenu, (i == 0 ? TRUE : FALSE), CountryGroup);
		g_signal_connect((gpointer)CountryItem, "activate",
				 G_CALLBACK(on_CPUMenu_Country_activate), GINT_TO_POINTER(i - 1));
	}
	
	// Separator
	NewMenuSeparator(CPUMenu_Country_SubMenu_Separator, "CPUMenu_Country_SubMenu_Separator", SubMenu);
	
	// Add the Auto-Detection Order configuration option.
	NewMenuItem(CPUMenu_Country_SubMenu_AutoDetectOrder, "Auto-Detection Order...", "Auto-Detection Order...", SubMenu);
	AddMenuCallback(CPUMenu_Country_SubMenu_AutoDetectOrder, on_CPUMenu_Country_SubMenu_AutoDetectOrder_activate);	
}


/**
 * create_gens_window_SoundMenu(): Create the Sound menu.
 * @param container Container for this menu.
 */
static void create_gens_window_SoundMenu(GtkWidget *container)
{
	GtkWidget *Sound;
	GtkWidget *SoundMenu;
	GtkWidget *SoundMenu_Enable;
	GtkWidget *SoundMenu_Separator1;
	GtkWidget *SoundMenu_Rate;
	GtkWidget *SoundMenu_Stereo;
	GtkWidget *SoundMenu_Separator2;
	GtkWidget *SoundMenu_Z80;
	GtkWidget *SoundMenu_Separator3;
	GtkWidget *SoundMenu_YM2612;
	GtkWidget *SoundMenu_YM2612_Improved;
	GtkWidget *SoundMenu_DAC;
	GtkWidget *SoundMenu_DAC_Improved;
	GtkWidget *SoundMenu_PSG;
	GtkWidget *SoundMenu_PSG_Improved;
	GtkWidget *SoundMenu_PCM;
	GtkWidget *SoundMenu_PWM;
	GtkWidget *SoundMenu_CDDA;
	GtkWidget *SoundMenu_Separator4;
	GtkWidget *SoundMenu_WAVDump;
	GtkWidget *SoundMenu_GYMDump;
	
	// Sound
	NewMenuItem(Sound, "_Sound", "Sound", container);
	
	// Menu object for the SoundMenu
	SoundMenu = gtk_menu_new();
	gtk_widget_set_name(SoundMenu, "SoundMenu");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(Sound), SoundMenu);
	
	// Enable
	NewMenuItem_Check(SoundMenu_Enable, "_Enable", "SoundMenu_Enable", SoundMenu, TRUE);
	AddMenuCallback(SoundMenu_Enable, on_SoundMenu_Enable_activate);
	
	// Separator
	NewMenuSeparator(SoundMenu_Separator1, "SoundMenu_Separator1", SoundMenu);

	// Rate
	NewMenuItem(SoundMenu_Rate, "_Rate", "SoundMenu_Rate", SoundMenu);
	// Rate submenu
	create_gens_window_SoundMenu_Rate_SubMenu(SoundMenu_Rate);
	
	// Stereo
	NewMenuItem_Check(SoundMenu_Stereo, "_Stereo", "SoundMenu_Stereo", SoundMenu, TRUE);
	AddMenuCallback(SoundMenu_Stereo, on_SoundMenu_Stereo_activate);
	
	// Separator
	NewMenuSeparator(SoundMenu_Separator2, "SoundMenu_Separator2", SoundMenu);
	
	// Z80
	NewMenuItem_Check(SoundMenu_Z80, "_Z80", "SoundMenu_Z80", SoundMenu, TRUE);
	AddMenuCallback(SoundMenu_Z80, on_SoundMenu_Z80_activate);
	
	// Separator
	NewMenuSeparator(SoundMenu_Separator3, "SoundMenu_Separator3", SoundMenu);
	
	// YM2612
	NewMenuItem_Check(SoundMenu_YM2612, "_YM2612", "SoundMenu_YM2612", SoundMenu, TRUE);
	AddMenuCallback(SoundMenu_YM2612, on_SoundMenu_YM2612_activate);
	
	// YM2612 Improved
	NewMenuItem_Check(SoundMenu_YM2612_Improved, "YM2612 Improved", "SoundMenu_YM2612_Improved", SoundMenu, FALSE);
	AddMenuCallback(SoundMenu_YM2612_Improved, on_SoundMenu_YM2612_Improved_activate);
	
	// DAC
	NewMenuItem_Check(SoundMenu_DAC, "_DAC", "SoundMenu_DAC", SoundMenu, TRUE);
	AddMenuCallback(SoundMenu_DAC, on_SoundMenu_DAC_activate);
	
	// DAC Improved
	NewMenuItem_Check(SoundMenu_DAC_Improved, "DAC Improved", "SoundMenu_DAC_Improved", SoundMenu, FALSE);
	AddMenuCallback(SoundMenu_DAC_Improved, on_SoundMenu_DAC_Improved_activate);
	
	// PSG
	NewMenuItem_Check(SoundMenu_PSG, "_PSG", "SoundMenu_PSG", SoundMenu, TRUE);
	AddMenuCallback(SoundMenu_PSG, on_SoundMenu_PSG_activate);
	
	// PSG Improved
	NewMenuItem_Check(SoundMenu_PSG_Improved, "PSG Improved", "SoundMenu_PSG_Improved", SoundMenu, FALSE);
	AddMenuCallback(SoundMenu_PSG_Improved, on_SoundMenu_PSG_Improved_activate);
	
	// PCM
	NewMenuItem_Check(SoundMenu_PCM, "P_CM", "SoundMenu_PCM", SoundMenu, TRUE);
	AddMenuCallback(SoundMenu_PCM, on_SoundMenu_PCM_activate);
	
	// PWM
	NewMenuItem_Check(SoundMenu_PWM, "P_WM", "SoundMenu_PWM", SoundMenu, TRUE);
	AddMenuCallback(SoundMenu_PWM, on_SoundMenu_PWM_activate);
	
	// CDDA
	NewMenuItem_Check(SoundMenu_CDDA, "CDD_A (CD Audio)", "SoundMenu_CDDA", SoundMenu, TRUE);
	AddMenuCallback(SoundMenu_CDDA, on_SoundMenu_CDDA_activate);
	
	// Separator
	NewMenuSeparator(SoundMenu_Separator4, "SoundMenu_Separator4", SoundMenu);
	
	// WAV Dump
	NewMenuItem(SoundMenu_WAVDump, "Start WAV Dump", "SoundMenu_WAVDump", SoundMenu);
	AddMenuCallback(SoundMenu_WAVDump, on_SoundMenu_WAVDump_activate);
	
	// GYM Dump
	NewMenuItem(SoundMenu_GYMDump, "Start GYM Dump", "SoundMenu_GYMDump", SoundMenu);
	AddMenuCallback(SoundMenu_GYMDump, on_SoundMenu_GYMDump_activate);
}


/**
 * create_gens_window_SoundMenu_Rate_SubMenu(): Create the Sound, Rate submenu.
 * @param container Container for this menu.
 */
static void create_gens_window_SoundMenu_Rate_SubMenu(GtkWidget *container)
{
	GtkWidget *SubMenu;
	GtkWidget *SndItem;
	GSList *SndGroup = NULL;
	
	// Sample rates are referenced by an index.
	// The index is not sorted by rate; the xx000 rates are 3, 4, 5.
	// This is probably for backwards-compatibilty with older Gens.
	const int SndRates[6][2] =
	{
		{0, 11025}, {3, 16000}, {1, 22050},
		{4, 32000}, {2, 44100}, {5, 48000},
	};
	
	int i;
	char SndName[16];
	char ObjName[64];
	
	// Create the submenu.
	SubMenu = gtk_menu_new();
	gtk_widget_set_name(SubMenu, "SoundMenu_Rate_SubMenu");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(container), SubMenu);
	
	// Create the rate entries.
	for (i = 0; i < 6; i++)
	{
		sprintf(SndName, "%d Hz", SndRates[i][1]);
		sprintf(ObjName, "SoundMenu_Rate_SubMenu_%d", SndRates[i][1]);
		NewMenuItem_Radio(SndItem, SndName, ObjName, SubMenu, (SndRates[i][1] == 22050 ? TRUE : FALSE), SndGroup);
		g_signal_connect((gpointer)SndItem, "activate",
				 G_CALLBACK(on_SoundMenu_Rate_SubMenu_activate),
				 GINT_TO_POINTER(SndRates[i][0]));
	}
}


/**
 * create_gens_window_OptionsMenu(): Create the Options menu.
 * @param container Container for this menu.
 */
static void create_gens_window_OptionsMenu(GtkWidget *container)
{
	GtkWidget *Options;
	GtkWidget *OptionsMenu;
	GtkWidget *OptionsMenu_GeneralOptions;	GtkWidget *OptionsMenu_GeneralOptions_Icon;
	GtkWidget *OptionsMenu_Joypads;		GtkWidget *OptionsMenu_Joypads_Icon;
	GtkWidget *OptionsMenu_Directories;	GtkWidget *OptionsMenu_Directories_Icon;
	GtkWidget *OptionsMenu_BIOSMiscFiles;	GtkWidget *OptionsMenu_BIOSMiscFiles_Icon;
	GtkWidget *OptionsMenu_Separator1;
#ifdef GENS_CDROM
	GtkWidget *OptionsMenu_CurrentCDDrive;	GtkWidget *OptionsMenu_CurrentCDDrive_Icon;
#endif
	GtkWidget *OptionsMenu_SegaCDSRAMSize;	GtkWidget *OptionsMenu_SegaCDSRAMSize_Icon;
	GtkWidget *OptionsMenu_Separator2;
	GtkWidget *OptionsMenu_LoadConfig;	GtkWidget *OptionsMenu_LoadConfig_Icon;
	GtkWidget *OptionsMenu_SaveConfigAs;	GtkWidget *OptionsMenu_SaveConfigAs_Icon;
	
	// Options
	NewMenuItem(Options, "_Options", "Options", container);
	
	// Menu object for the OptionsMenu
	OptionsMenu = gtk_menu_new();
	gtk_widget_set_name(OptionsMenu, "OptionsMenu");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(Options), OptionsMenu);
	
	// General Options
	NewMenuItem_Icon(OptionsMenu_GeneralOptions, "_General Options...", "OptionsMenu_GeneralOptions", OptionsMenu,
			 OptionsMenu_GeneralOptions_Icon, "ksysguard.png");
	AddMenuCallback(OptionsMenu_GeneralOptions, on_OptionsMenu_GeneralOptions_activate);
	
	// Joypads
	NewMenuItem_Icon(OptionsMenu_Joypads, "_Joypads...", "OptionsMenu_Joypads", OptionsMenu,
			 OptionsMenu_Joypads_Icon, "package_games.png");
	AddMenuCallback(OptionsMenu_Joypads, on_OptionsMenu_Joypads_activate);
	
	// Directories
	NewMenuItem_Icon(OptionsMenu_Directories, "_Directories...", "OptionsMenu_Directories", OptionsMenu,
			 OptionsMenu_Directories_Icon, "folder_slin_open.png");
	AddMenuCallback(OptionsMenu_Directories, on_OptionsMenu_Directories_activate);
	
	// BIOS/Misc Files...
	NewMenuItem_Icon(OptionsMenu_BIOSMiscFiles, "_BIOS/Misc Files...", "OptionsMenu_BIOSMiscFiles", OptionsMenu,
			 OptionsMenu_BIOSMiscFiles_Icon, "binary.png");
	AddMenuCallback(OptionsMenu_BIOSMiscFiles, on_OptionsMenu_BIOSMiscFiles_activate);
	
	// Separator
	NewMenuSeparator(OptionsMenu_Separator1, "OptionsMenu_Separator1", OptionsMenu);
	
#ifdef GENS_CDROM
	// Current CD Drive...
	NewMenuItem_StockIcon(OptionsMenu_CurrentCDDrive, "Current _CD Drive...", "OptionsMenu_CurrentCDDrive", OptionsMenu,
			      OptionsMenu_CurrentCDDrive_Icon, "gtk-cdrom");
	AddMenuCallback(OptionsMenu_CurrentCDDrive, on_OptionsMenu_CurrentCDDrive_activate);
#endif
	
	// Sega CD SRAM Size
	NewMenuItem_Icon(OptionsMenu_SegaCDSRAMSize, "Sega CD S_RAM Size", "OptionsMenu_SegaCDSRAMSize", OptionsMenu,
			 OptionsMenu_SegaCDSRAMSize_Icon, "memory.png");
	// Sega CD SRAM Size submenu
	create_gens_window_OptionsMenu_SegaCDSRAMSize_SubMenu(OptionsMenu_SegaCDSRAMSize);
	
	// Separator
	NewMenuSeparator(OptionsMenu_Separator2, "OptionsMenu_Separator2", OptionsMenu);
	
	// Load Config...
	NewMenuItem_StockIcon(OptionsMenu_LoadConfig, "_Load Config...", "OptionsMenu_LoadConfig", OptionsMenu,
			      OptionsMenu_LoadConfig_Icon, "gtk-open");
	AddMenuCallback(OptionsMenu_LoadConfig, on_OptionsMenu_LoadConfig_activate);
	
	// Save Config As...
	NewMenuItem_StockIcon(OptionsMenu_SaveConfigAs, "_Save Config As...", "OptionsMenu_SaveConfigAs", OptionsMenu,
			      OptionsMenu_SaveConfigAs_Icon, "gtk-save-as");
	AddMenuCallback(OptionsMenu_SaveConfigAs, on_OptionsMenu_SaveConfigAs_activate);
}


/**
 * create_gens_window_OptionsMenu_SegaCDSRAMSize_SubMenu(): Create the Options, Sega CD SRAM Size submenu.
 * @param container Container for this menu.
 */
static void create_gens_window_OptionsMenu_SegaCDSRAMSize_SubMenu(GtkWidget *container)
{
	GtkWidget *SubMenu;
	GtkWidget *SRAMItem;
	GSList *SRAMGroup = NULL;
	
	int i;
	char SRAMName[16];
	char ObjName[64];
	
	// Create the submenu.
	SubMenu = gtk_menu_new();
	gtk_widget_set_name(SubMenu, "OptionsMenu_SegaCDSRAMSize_SubMenu");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(container), SubMenu);
	
	// Create the rate entries.
	for (i = -1; i <= 3; i++)
	{
		if (i == -1)
		{
			strcpy(SRAMName, "None");
			strcpy(ObjName, "OptionsMenu_SegaCDSRAMSize_SubMenu_None");
		}
		else
		{
			sprintf(SRAMName, "%d KB", 8 << i);	
			sprintf(ObjName, "OptionsMenu_SegaCDSRAMSize_SubMenu_%d", i);
		}
		NewMenuItem_Radio(SRAMItem, SRAMName, ObjName, SubMenu, (i == -1 ? TRUE : FALSE), SRAMGroup);
		g_signal_connect((gpointer)SRAMItem, "activate",
				 G_CALLBACK(on_OptionsMenu_SegaCDSRAMSize_SubMenu_activate),
				 GINT_TO_POINTER(i));
	}
}


/**
 * create_gens_window_HelpMenu(): Create the Help menu.
 * @param container Container for this menu.
 */
static void create_gens_window_HelpMenu(GtkWidget *container)
{
	GtkWidget *Help;
	GtkWidget *HelpMenu;
	GtkWidget *HelpMenu_About;		GtkWidget *HelpMenu_About_Icon;
	
	// Help
	NewMenuItem(Help, "_Help", "Help", container);
	
	// Menu object for the HelpMenu
	HelpMenu = gtk_menu_new();
	gtk_widget_set_name(HelpMenu, "HelpMenu");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(Help), HelpMenu);
	
	// About
	NewMenuItem_StockIcon(HelpMenu_About, "_About", "About", HelpMenu, HelpMenu_About_Icon, "gtk-help");
	AddMenuCallback(HelpMenu_About, on_HelpMenu_About_activate);
}
