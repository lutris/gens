/**
 * GENS: Main Window. (GTK+)
 */

#include "genswindow.h"
#include "genswindow_callbacks.h"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>

// GENS GTK+ miscellaneous functions
#include "gtk-misc.h"

#include "gens.h"
#include "debug.h"

GtkWidget *gens_window;
GtkWidget *MenuBar;

#ifdef GENS_DEBUG
// Debug menu items
GtkWidget *debugMenuItems[9];
GtkWidget *debugSeparators[2];
#endif

GtkAccelGroup *accel_group;
GtkTooltips *tooltips;

void create_genswindow_menubar(GtkWidget *container);
void create_genswindow_FileMenu(GtkWidget *container);
void create_genswindow_FileMenu_ChangeStateSubMenu(GtkWidget *container);
void create_genswindow_GraphicsMenu(GtkWidget *container);
void create_genswindow_GraphicsMenu_OpenGLResSubMenu(GtkWidget *container);
void create_genswindow_GraphicsMenu_bppSubMenu(GtkWidget *container);
void create_genswindow_GraphicsMenu_RenderSubMenu(GtkWidget *container);
void create_genswindow_GraphicsMenu_FrameSkipSubMenu(GtkWidget *container);
void create_genswindow_CPUMenu(GtkWidget *container);
#ifdef GENS_DEBUG
void create_genswindow_CPUMenu_DebugSubMenu(GtkWidget *container);
#endif
void create_genswindow_SoundMenu(GtkWidget *container);
void create_genswindow_HelpMenu(GtkWidget *container);

// Set to 0 to temporarily disable callbacks.
int do_callbacks = 1;


// Macros from Glade used to store GtkWidget pointers.
#define GLADE_HOOKUP_OBJECT(component, widget, name)			\
{									\
	g_object_set_data_full(G_OBJECT(component), name,		\
			       gtk_widget_ref(widget),			\
			       (GDestroyNotify)gtk_widget_unref);	\
}


#define GLADE_HOOKUP_OBJECT_NO_REF(component, widget, name)		\
{									\
	g_object_set_data(G_OBJECT(component), name, widget);		\
};


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
	IconWidget = create_pixmap(gens_window, IconFilename);					\
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
			 G_CALLBACK (CallbackFunction), NULL);					\
}


GtkWidget* create_gens_window(void)
{
	GdkPixbuf *gens_window_icon_pixbuf;
	GtkWidget *vbox1;
	GtkWidget *handlebox1;
	
	tooltips = gtk_tooltips_new();
	accel_group = gtk_accel_group_new();
	
	// Create the GENS window.
	gens_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_widget_set_name(gens_window, "gens_window");
	gtk_container_set_border_width(GTK_CONTAINER(gens_window), 5);
	gtk_window_set_title(GTK_WINDOW(gens_window), "Gens");
	gtk_window_set_position(GTK_WINDOW(gens_window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(gens_window), 640, 480);
	GLADE_HOOKUP_OBJECT_NO_REF(gens_window, gens_window, "gens_window");
	
	// Load the window icon.
	gens_window_icon_pixbuf = create_pixbuf("Gens2.ico");
	if (gens_window_icon_pixbuf)
	{
		gtk_window_set_icon(GTK_WINDOW (gens_window), gens_window_icon_pixbuf);
		gdk_pixbuf_unref(gens_window_icon_pixbuf);
	}
	
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
	create_genswindow_menubar(handlebox1);
	
	// Add the accel group.
	gtk_window_add_accel_group(GTK_WINDOW(gens_window), accel_group);
	
	// Callbacks for if the window is closed.
	g_signal_connect ((gpointer)gens_window, "delete_event",
			  G_CALLBACK(on_gens_window_close), NULL);
	g_signal_connect ((gpointer)gens_window, "destroy_event",
			  G_CALLBACK(on_gens_window_close), NULL);
	
	return gens_window;
}


/**
 * create_genswindow_menubar(): Create the menu bar.
 * @param container Container for the menu bar.
 */
void create_genswindow_menubar(GtkWidget *container)
{
	MenuBar = gtk_menu_bar_new();
	gtk_widget_set_name(MenuBar, "MenuBar");
	gtk_widget_show(MenuBar);
	gtk_container_add(GTK_CONTAINER(container), MenuBar);
	
	// Menus
	create_genswindow_FileMenu(MenuBar);
	create_genswindow_GraphicsMenu(MenuBar);
	create_genswindow_CPUMenu(MenuBar);
	create_genswindow_SoundMenu(MenuBar);
	create_genswindow_HelpMenu(MenuBar);
}


/**
 * create_genswindow_FileMenu(): Create the File menu.
 * @param container Container for this menu.
 */
void create_genswindow_FileMenu(GtkWidget *container)
{
	GtkWidget *File;			GtkWidget *File_Icon;
	GtkWidget *FileMenu;
	GtkWidget *FileMenu_OpenROM;		GtkWidget *FileMenu_OpenROM_Icon;
	GtkWidget *FileMenu_BootCD;		GtkWidget *FileMenu_BootCD_Icon;
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
	GtkWidget *FileMenu_Quit;		GtkWidget *FileMenu_Quit_Icon;
	
	// File
	NewMenuItem_Icon(File, "_File", "File", container, File_Icon, "resource.png");
	
	// Menu object for the FileMenu
	FileMenu = gtk_menu_new();
	gtk_widget_set_name(FileMenu, "FileMenu");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(File), FileMenu);
	
	// Open ROM...
	NewMenuItem_Icon(FileMenu_OpenROM, "_Open ROM...", "FileMenu_OpenROM", FileMenu,
			 FileMenu_OpenROM_Icon, "folder_slin_open.png");
	AddMenuAccelerator(FileMenu_OpenROM, GDK_O, GDK_CONTROL_MASK);
	AddMenuCallback(FileMenu_OpenROM, on_FileMenu_OpenROM_activate);
	
	// Boot CD
	NewMenuItem_Icon(FileMenu_BootCD, "_Boot CD", "FileMenu_BootCD", FileMenu,
			 FileMenu_BootCD_Icon, "cdrom2_unmount.png");
	AddMenuAccelerator(FileMenu_BootCD, GDK_B, GDK_CONTROL_MASK);
	AddMenuCallback(FileMenu_BootCD, on_FileMenu_BootCD_activate);
	
	// Netplay (currently disabled)
	NewMenuItem_Icon(FileMenu_Netplay, "_Netplay", "FileMenu_Netplay", FileMenu,
			 FileMenu_Netplay_Icon, "Modem.png");
	gtk_widget_set_sensitive(FileMenu_Netplay, FALSE);
	
	// ROM History
	NewMenuItem_Icon(FileMenu_ROMHistory, "ROM _History", "FileMenu_ROMHistory", FileMenu,
			 FileMenu_ROMHistory_Icon, "history.png");
	// TODO: ROM History submenu
	
	// Close ROM
	NewMenuItem_Icon(FileMenu_CloseROM, "_Close ROM", "FileMenu_CloseROM", FileMenu,
			 FileMenu_CloseROM_Icon, "filleclose.png");
	gtk_widget_add_accelerator(FileMenu_CloseROM, "activate", accel_group,
				   GDK_W, (GdkModifierType)GDK_CONTROL_MASK,
				   GTK_ACCEL_VISIBLE);
	AddMenuCallback(FileMenu_CloseROM, on_FileMenu_CloseROM_activate);
	
	// Separator
	NewMenuSeparator(FileMenu_Separator1, "FileMenu_Separator1", FileMenu);
	
	// Game Genie
	NewMenuItem_Icon(FileMenu_GameGenie, "_Game Genie", "FileMenu_GameGenie", FileMenu,
			 FileMenu_GameGenie_Icon, "password.png");
	AddMenuCallback(FileMenu_GameGenie, on_FileMenu_GameGenie_activate);
	
	// Separator
	NewMenuSeparator(FileMenu_Separator2, "FileMenu_Separator2", FileMenu);
	
	// Load State...
	NewMenuItem_Icon(FileMenu_LoadState, "_Load State...", "FileMenu_LoadState", FileMenu,
			 FileMenu_LoadState_Icon, "folder_slin_open.png");
	AddMenuAccelerator(FileMenu_LoadState, GDK_F8, GDK_SHIFT_MASK);
	AddMenuCallback(FileMenu_LoadState, on_FileMenu_LoadState_activate);
	
	// Save State As...
	NewMenuItem_StockIcon(FileMenu_SaveState, "_Save State As...", "FileMenu_SaveState", FileMenu,
			      FileMenu_SaveState_Icon, "gtk-save-as");
	AddMenuAccelerator(FileMenu_SaveState, GDK_F5, GDK_SHIFT_MASK);
	AddMenuCallback(FileMenu_SaveState, on_FileMenu_SaveState_activate);
	
	// Quick Load
	NewMenuItem_Icon(FileMenu_QuickLoad, "Quick Load", "FileMenu_QuickLoad", FileMenu,
			 FileMenu_QuickLoad_Icon, "reload.png");
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
	create_genswindow_FileMenu_ChangeStateSubMenu(FileMenu_ChangeState);
	
	// Quit
	NewMenuItem_StockIcon(FileMenu_Quit, "_Quit", "FileMenu_Quit", FileMenu,
			      FileMenu_Quit_Icon, "gtk-quit");
	AddMenuAccelerator(FileMenu_Quit, GDK_Q, GDK_CONTROL_MASK);
	AddMenuCallback(FileMenu_Quit, on_FileMenu_Quit_activate);
}


/**
 * genswindow_FileMenu_ChangeStateSubMenu(): Create the File, Change State submenu.
 * @param container Container for this menu.
 */
#define CREATE_SAVE_SLOT(widget, ObjName, text, defState, callbackFunc)			\
{											\
	NewMenuItem_Radio(widget, text, ObjName, SubMenu, defState, SlotGroup);		\
	g_signal_connect((gpointer)widget, "activate", G_CALLBACK(callbackFunc), NULL);	\
}
void create_genswindow_FileMenu_ChangeStateSubMenu(GtkWidget *container)
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
		sprintf(ObjName, "GraphicsMenu_OpenGLRes_SubMenu_%s", SlotName);
		NewMenuItem_Radio(SlotItem, SlotName, ObjName, SubMenu, (i == 0 ? TRUE : FALSE), SlotGroup);
		g_signal_connect((gpointer)SlotItem, "activate",
				 G_CALLBACK(on_FileMenu_ChangeState_SubMenu_SlotItem_activate),
				 GINT_TO_POINTER(i));
	}
}


/**
 * create_genswindow_GraphicsMenu(): Create the Graphics menu.
 * @param container Container for this menu.
 */
void create_genswindow_GraphicsMenu(GtkWidget *container)
{
	GtkWidget *Graphics;			GtkWidget *Graphics_Icon;
	GtkWidget *GraphicsMenu;
	GtkWidget *GraphicsMenu_FullScreen;	GtkWidget *GraphicsMenu_FullScreen_Icon;
	GtkWidget *GraphicsMenu_VSync;
	GtkWidget *GraphicsMenu_Stretch;
	GtkWidget *GraphicsMenu_Separator1;
	GtkWidget *GraphicsMenu_OpenGL;
	GtkWidget *GraphicsMenu_OpenGLRes;
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
	NewMenuItem_Icon(Graphics, "_Graphics", "Graphics", container, Graphics_Icon, "xpaint.png");
	
	// Menu object for the GraphicsMenu
	GraphicsMenu = gtk_menu_new();
	gtk_widget_set_name(GraphicsMenu, "GraphicsMenu");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(Graphics), GraphicsMenu);
	
	// Full Screen
	NewMenuItem_Icon(GraphicsMenu_FullScreen, "_Full Screen", "GraphicsMenu_FullScreen", GraphicsMenu,
			 GraphicsMenu_FullScreen_Icon, "viewmag1.png");
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
	
	// OpenGL
	NewMenuItem_Check(GraphicsMenu_OpenGL, "Open_GL", "GraphicsMenu_OpenGL", GraphicsMenu, FALSE);
	AddMenuAccelerator(GraphicsMenu_OpenGL, GDK_r, GDK_SHIFT_MASK);
	AddMenuCallback(GraphicsMenu_OpenGL, on_GraphicsMenu_OpenGL_activate);
	
	// OpenGL Resolution
	NewMenuItem(GraphicsMenu_OpenGLRes, "OpenGL Resolution", "GraphicsMenu_OpenGLRes", GraphicsMenu);
	// OpenGL Resolution submenu
	create_genswindow_GraphicsMenu_OpenGLResSubMenu(GraphicsMenu_OpenGLRes);
	
	// Bits per pixel (OpenGL mode) [TODO: Where is this value actually used?]
	NewMenuItem(GraphicsMenu_bpp, "Bits per pixel", "GraphicsMenu_bpp", GraphicsMenu);
	// Bits per pixel submenu
	create_genswindow_GraphicsMenu_bppSubMenu(GraphicsMenu_bpp);
	
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
	create_genswindow_GraphicsMenu_RenderSubMenu(GraphicsMenu_Render);
	
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
	create_genswindow_GraphicsMenu_FrameSkipSubMenu(GraphicsMenu_FrameSkip);
	
	// Separator
	NewMenuSeparator(GraphicsMenu_Separator5, "GraphicsMenu_Separator5", GraphicsMenu);
	
	// Screen Shot
	NewMenuItem_Icon(GraphicsMenu_ScreenShot, "Screen Shot", "GraphicsMenu_ScreenShot", GraphicsMenu,
			 GraphicsMenu_ScreenShot_Icon, "editcopy.png");
	AddMenuAccelerator(GraphicsMenu_ScreenShot, GDK_BackSpace, GDK_SHIFT_MASK);
	AddMenuCallback(GraphicsMenu_ScreenShot, on_GraphicsMenu_ScreenShot_activate);
}


/**
 * create_genswindow_GraphicsMenu_OpenGLResSubMenu(): Create the Graphics, OpenGL Resolution submenu.
 * @param container Container for this menu.
 */
void create_genswindow_GraphicsMenu_OpenGLResSubMenu(GtkWidget *container)
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


/**
 * create_genswindow_GraphicsMenu_bppSubMenu(): Create the Graphics, Bits per pixel submenu.
 * @param container Container for this menu.
 */
void create_genswindow_GraphicsMenu_bppSubMenu(GtkWidget *container)
{
	GtkWidget *SubMenu;
	GtkWidget *bppItem;
	GSList *bppGroup = NULL;
	
	// TODO: Move this array somewhere else.
	int bpp[3] = {16, 24, 32};
	
	int i;
	char ObjName[64];
	char bppName[8];
	
	// Create the submenu.
	SubMenu = gtk_menu_new();
	gtk_widget_set_name(SubMenu, "GraphicsMenu_bpp_SubMenu");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(container), SubMenu);
	
	// Create the bits per pixel entries.
	for (i = 0; i < 3; i++)
	{
		sprintf(bppName, "%d", bpp[i]);
		sprintf(ObjName, "GraphicsMenu_bpp_SubMenu_%s", bppName);
		NewMenuItem_Radio(bppItem, bppName, ObjName, SubMenu, (i == 0 ? TRUE : FALSE), bppGroup);
		g_signal_connect((gpointer)bppItem, "activate",
				 G_CALLBACK(on_GraphicsMenu_bpp_SubMenu_bppItem_activate),
				 GINT_TO_POINTER(bpp[i]));
	}
}


/**
 * create_genswindow_GraphicsMenu_RenderSubMenu(): Create the Graphics, Render submenu.
 * @param container Container for this menu.
 */
void create_genswindow_GraphicsMenu_RenderSubMenu(GtkWidget *container)
{
	GtkWidget *SubMenu;
	GtkWidget *RenderItem;
	GSList *RenderGroup = NULL;
	
	// TODO: Move this array somewhere else.
	const char* Render[12] =
	{
		"Normal",
		"Double",
		"Interpolated",
		"Scanline",
		"50% Scanline",
		"25% Scanline",
		"Interpolated Scanline",
		"Interpolated 50% Scanline",
		"Interpolated 25% Scanline",
		"2xSAI (Kreed)",
		"Scale2x",
		"Hq2x",
	};
	const char* RenderTag[12] =
	{
		"normal",
		"double",
		"int",
		"scan",
		"50scan",
		"25scan",
		"intscan",
		"int50scan",
		"int25scan",
		"2xsai",
		"scale2x",
		"hq2x",
	};
	
	int i;
	char ObjName[64];
	
	// Create the submenu.
	SubMenu = gtk_menu_new();
	gtk_widget_set_name(SubMenu, "GraphicsMenu_Render_SubMenu");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(container), SubMenu);
	
	// Create the render entries.
	for (i = 0; i < 12; i++)
	{
		sprintf(ObjName, "GraphicsMenu_Render_SubMenu_%s", RenderTag[i]);
		NewMenuItem_Radio(RenderItem, Render[i], ObjName, SubMenu, (i == 0 ? TRUE : FALSE), RenderGroup);
		g_signal_connect((gpointer)RenderItem, "activate",
				 G_CALLBACK(on_GraphicsMenu_Render_SubMenu_RenderItem_activate),
				 GINT_TO_POINTER(i + 1));
	}
}


/**
 * create_genswindow_GraphicsMenu_FrameSkipSubMenu(): Create the Graphics, Frame Skip submenu.
 * @param container Container for this menu.
 */
void create_genswindow_GraphicsMenu_FrameSkipSubMenu(GtkWidget *container)
{
	GtkWidget *SubMenu;
	GtkWidget *FSItem;
	GSList *FSGroup = NULL;
	
	int i;
	char ObjName[64];
	char FSName[8];
	
	// Create the submenu.
	SubMenu = gtk_menu_new();
	gtk_widget_set_name(SubMenu, "GraphicsMenu_bpp_FrameSkip_SubMenu");
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
 * create_genswindow_CPUMenu(): Create the CPU menu.
 * @param container Container for this menu.
 */
void create_genswindow_CPUMenu(GtkWidget *container)
{
	GtkWidget *CPU;				GtkWidget *CPU_Icon;
	GtkWidget *CPUMenu;
	#ifdef GENS_DEBUG
	GtkWidget *CPUMenu_Debug;
	GtkWidget *CPUMenu_Separator1;
	#endif
	GtkWidget *CPUMenu_Country;
	GtkWidget *CPUMenu_HardReset;		GtkWidget *CPUMenu_HardReset_Icon;
	GtkWidget *CPUMenu_Reset68000;
	GtkWidget *CPUMenu_ResetMain68000;
	GtkWidget *CPUMenu_ResetSub68000;
	GtkWidget *CPUMenu_ResetMainSH2;
	GtkWidget *CPUMenu_ResetSubSH2;
	GtkWidget *CPUMenu_ResetZ80;
	GtkWidget *CPUMenu_Separator2;
	GtkWidget *CPUMenu_SegaCD_PerfectSync;
	
	// CPU
	NewMenuItem_Icon(CPU, "_CPU", "CPU", container, CPU_Icon, "memory.png");
	
	// Menu object for the CPUMenu
	CPUMenu = gtk_menu_new();
	gtk_widget_set_name(CPUMenu, "CPUMenu");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(CPU), CPUMenu);
	
	#ifdef GENS_DEBUG
	// Debug
	NewMenuItem(CPUMenu_Debug, "_Debug", "CPUMenu_Debug", CPUMenu);
	// Debug submenu
	create_genswindow_CPUMenu_DebugSubMenu(CPUMenu_Debug);
	
	// Separator
	NewMenuSeparator(CPUMenu_Separator1, "CPUMenu_Separator1", CPUMenu);
	#endif
	
	// Country
	NewMenuItem(CPUMenu_Country, "_Country", "CPUMenu_Country", CPUMenu);
	// Country submenu
	// TODO
	
	// Hard Reset
	NewMenuItem_Icon(CPUMenu_HardReset, "Hard _Reset", "CPUMenu_HardReset", CPUMenu,
			 CPUMenu_HardReset_Icon, "reload.png");
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
	NewMenuSeparator(CPUMenu_Separator2, "CPUMenu_Separator2", CPUMenu);
	
	// SegaCD Perfect Sync
	NewMenuItem_Check(CPUMenu_SegaCD_PerfectSync, "SegaCD Perfect Sync (SLOW)", "SegaCD Perfect Sync (SLOW)", CPUMenu, FALSE);
	AddMenuCallback(CPUMenu_SegaCD_PerfectSync, on_CPUMenu_SegaCD_PerfectSync_activate);
}


#ifdef GENS_DEBUG
/**
 * create_genswindow_CPUMenu_DebugSubMenu(): Create the CPU, Debug submenu.
 * @param container Container for this menu.
 */
void create_genswindow_CPUMenu_DebugSubMenu(GtkWidget *container)
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
	const char* DebugTag[9] =
	{
		"md_68000",
		"md_Z80",
		"md_VDP",
		"mcd_68000",
		"mcd_CDC",
		"mcd_GFX",
		"32X_MSH2",
		"32X_SSH2",
		"32X_VDP",
	};
	
	int i;
	char ObjName[64];
	
	// Create the submenu.
	SubMenu = gtk_menu_new();
	gtk_widget_set_name(SubMenu, "CPUMenu_Debug_SubMenu");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(container), SubMenu);
	
	// Create the render entries.
	for (i = 0; i < 9; i++)
	{
		sprintf(ObjName, "CPUMenu_Debug_SubMenu_%s", DebugTag[i]);
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
#endif


/**
 * create_genswindow_SoundMenu(): Create the Sound menu.
 * @param container Container for this menu.
 */
void create_genswindow_SoundMenu(GtkWidget *container)
{
	GtkWidget *Sound;			GtkWidget *Sound_Icon;
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
	
	// Sound
	NewMenuItem_Icon(Sound, "_Sound", "Sound", container, Sound_Icon, "kmix.png");
	
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
	// TODO: Rate submenu
	
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
}


/**
 * create_genswindow_HelpMenu(): Create the Help menu.
 * @param container Container for this menu.
 */
void create_genswindow_HelpMenu(GtkWidget *container)
{
	GtkWidget *Help;			GtkWidget *Help_Icon;
	GtkWidget *HelpMenu;
	GtkWidget *HelpMenu_About;		GtkWidget *HelpMenu_About_Icon;
	
	// Help
	NewMenuItem_StockIcon(Help, "_Help", "Help", container, Help_Icon, "gtk-help");
	
	// Menu object for the HelpMenu
	HelpMenu = gtk_menu_new();
	gtk_widget_set_name(HelpMenu, "HelpMenu");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(Help), HelpMenu);
	
	// About
	NewMenuItem_StockIcon(HelpMenu_About, "_About", "About", HelpMenu, HelpMenu_About_Icon, "gtk-help");
	AddMenuCallback(HelpMenu_About, on_HelpMenu_About_activate);
}
