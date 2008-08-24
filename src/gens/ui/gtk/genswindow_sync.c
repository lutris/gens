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
#include "vdp_io.h"


// Per-menu synchronization functions.
void Sync_GensWindow_FileMenu(void);
void Sync_GensWindow_GraphicsMenu(void);
void Sync_GensWindow_CPUMenu(void);


/**
 * Sync_GensWindow(): Synchronize the GENS Main Window.
 */
void Sync_GensWindow(WindowSyncType sync)
{
	// Disable callbacks so things don't get screwed up.
	do_callbacks = 0;
	
	Sync_GensWindow_FileMenu();
	Sync_GensWindow_GraphicsMenu();
	Sync_GensWindow_CPUMenu();
	
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


/**
 * Sync_GensWindow_CPUMenu(): Synchronize the CPU menu.
 */
void Sync_GensWindow_CPUMenu(void)
{
	// TODO: Debug
	// TODO: Country
	
	// Hide and show appropriate RESET items.
	GtkWidget *MItem_Reset68K, *MItem_ResetM68K, *MItem_ResetS68K;
	GtkWidget *MItem_ResetMSH2, *MItem_ResetSSH2;
	
	MItem_Reset68K = lookup_widget(gens_window, "CPUMenu_Reset68000");
	MItem_ResetM68K = lookup_widget(gens_window, "CPUMenu_ResetMain68000");
	MItem_ResetS68K = lookup_widget(gens_window, "CPUMenu_ResetSub68000");
	MItem_ResetMSH2 = lookup_widget(gens_window, "CPUMenu_ResetMainSH2");
	MItem_ResetSSH2 = lookup_widget(gens_window, "CPUMenu_ResetSubSH2");
	
	if (SegaCD_Started)
	{
		// SegaCD: Hide regular 68000; show Main 68000 and Sub 68000.
		gtk_widget_hide(MItem_Reset68K);
		gtk_widget_show(MItem_ResetM68K);
		gtk_widget_show(MItem_ResetS68K);
	}
	else
	{
		// No SegaCD: Show regular 68000; hide Main 68000 and Sub 68000;
		gtk_widget_show(MItem_Reset68K);
		gtk_widget_hide(MItem_ResetM68K);
		gtk_widget_hide(MItem_ResetS68K);
	}
	
	if (_32X_Started)
	{
		// 32X: Show Main SH2 and Sub SH2.
		gtk_widget_show(MItem_ResetMSH2);
		gtk_widget_show(MItem_ResetSSH2);
	}
	else
	{
		// 32X: Hide Main SH2 and Sub SH2.
		gtk_widget_hide(MItem_ResetMSH2);
		gtk_widget_hide(MItem_ResetSSH2);
	}
}
