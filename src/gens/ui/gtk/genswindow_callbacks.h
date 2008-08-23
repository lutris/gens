/**
 * GENS: Main Window Callbacks. (GTK+)
 */


#ifndef UI_GENSWINDOW_CALLBACKS_H
#define UI_GENSWINDOW_CALLBACKS_H

#ifdef __cplusplus
extern "C" {
#endif


#include <gtk/gtk.h>


// Window is closed.
gboolean on_gens_window_close(GtkMenuItem *menuitem, GdkEvent *event, gpointer user_data);


// File menu
void on_FileMenu_OpenROM_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_FileMenu_BootCD_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_FileMenu_CloseROM_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_FileMenu_GameGenie_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_FileMenu_LoadState_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_FileMenu_SaveState_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_FileMenu_QuickLoad_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_FileMenu_QuickSave_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_FileMenu_ChangeState_SubMenu_SlotItem_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_FileMenu_Quit_activate(GtkMenuItem *menuitem, gpointer user_data);


// Graphics menu
void on_GraphicsMenu_FullScreen_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_GraphicsMenu_VSync_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_GraphicsMenu_Stretch_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_GraphicsMenu_OpenGL_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_GraphicsMenu_OpenGLRes_SubMenu_ResItem_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_GraphicsMenu_bpp_SubMenu_bppItem_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_GraphicsMenu_ColorAdjust_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_GraphicsMenu_SpriteLimit_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_GraphicsMenu_ScreenShot_activate(GtkMenuItem *menuitem, gpointer user_data);


#ifdef __cplusplus
}
#endif

#endif
