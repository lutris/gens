/**
 * GENS: (GTK+) Main Window Synchronization.
 */


#include "genswindow.h"
#include "genswindow_sync.h"
#include "gtk-misc.h"

#include "g_main.h"
#include "g_sdldraw.h"
#include "vdp_rend.h"
#include "gens.h"


void Sync_GensWindow_FileMenu(void);
void Sync_GensWindow_GraphicsMenu(void);


/**
 * Sync_GensWindow(): Synchronize the GENS Main Window.
 */
void Sync_GensWindow(WindowSyncType sync)
{
	// Disable callbacks so things don't get screwed up.
	do_callbacks = 0;
	
	Sync_GensWindow_FileMenu();
	Sync_GensWindow_GraphicsMenu();
	
	// Re-enable callbacks.
	do_callbacks = 1;
}


/**
 * Sync_GensWindow_FileMenu(): Synchronize the File Menu.
 */
void Sync_GensWindow_FileMenu(void)
{
	GtkWidget *MItem_SaveState;
	
	// TODO: ROM History
	
	// Current savestate
	sprintf(Str_Tmp, "FileMenu_ChangeState_SubMenu_%d", Current_State);
	MItem_SaveState = lookup_widget(gens_window, Str_Tmp);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_SaveState), TRUE);
	
	// TODO: Disable Close ROM if no ROM is loaded.
}


/**
 * Sync_GensWindow_GraphicsMenu(): Synchronize the Graphics menu.
 */
void Sync_GensWindow_GraphicsMenu(void)
{
	GtkWidget *MItem_VSync, *MItem_Stretch, *MItem_OpenGL, *MItem_SpriteLimit;
	GtkWidget *MItem_OpenGL_Resolution, *MItem_bpp, *MItem_Render, *MItem_FrameSkip;
	
	// Simple checkbox items
	MItem_VSync = lookup_widget(gens_window, "GraphicsMenu_VSync");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_VSync), W_VSync);
	MItem_Stretch = lookup_widget(gens_window, "GraphicsMenu_Stretch");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_Stretch), Stretch);
	MItem_OpenGL = lookup_widget(gens_window, "GraphicsMenu_OpenGL");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_OpenGL), Opengl);
	MItem_SpriteLimit = lookup_widget(gens_window, "GraphicsMenu_SpriteLimit");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_SpriteLimit), Sprite_Over);
	
	// OpenGL Resolution
	sprintf(Str_Tmp, "GraphicsMenu_OpenGLRes_SubMenu_%dx%d", Width_gl, Height_gl);
	MItem_OpenGL_Resolution = lookup_widget(gens_window, Str_Tmp);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_OpenGL_Resolution), TRUE);
	
	// Bits per pixel
	sprintf(Str_Tmp, "GraphicsMenu_bpp_SubMenu_%d", Bpp);
	MItem_bpp = lookup_widget(gens_window, Str_Tmp);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_bpp), TRUE);
	
	// Render
	sprintf(Str_Tmp, "GraphicsMenu_Render_SubMenu_%d", Render_Mode);
	MItem_Render = lookup_widget(gens_window, Str_Tmp);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_Render), TRUE);
	
	// Frame Skip
	if (Frame_Skip == -1)
		strcpy(Str_Tmp, "GraphicsMenu_FrameSkip_SubMenu_Auto");
	else
		sprintf(Str_Tmp, "GraphicsMenu_FrameSkip_SubMenu_%d", Frame_Skip);
	MItem_FrameSkip = lookup_widget(gens_window, Str_Tmp);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_FrameSkip), TRUE);
}
