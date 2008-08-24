/**
 * GENS: Main Window Callbacks. (GTK+)
 */


#ifndef GENSWINDOW_CALLBACKS_H
#define GENSWINDOW_CALLBACKS_H

#ifdef __cplusplus
extern "C" {
#endif


#include <gtk/gtk.h>
#include "gens.h"


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
void on_GraphicsMenu_Render_SubMenu_RenderItem_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_GraphicsMenu_FrameSkip_SubMenu_FSItem_activate(GtkMenuItem *menuitem, gpointer user_data);

// CPU menu
#ifdef GENS_DEBUG
void on_CPUMenu_Debug_SubMenu_activate(GtkMenuItem *menuitem, gpointer user_data);
#endif
void on_CPUMenu_HardReset_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_CPUMenu_ResetMain68000_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_CPUMenu_ResetSub68000_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_CPUMenu_ResetMainSH2_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_CPUMenu_ResetSubSH2_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_CPUMenu_ResetZ80_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_CPUMenu_SegaCD_PerfectSync_activate(GtkMenuItem *menuitem, gpointer user_data);

// Sound menu
void on_SoundMenu_Enable_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_SoundMenu_Rate_SubMenu_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_SoundMenu_Stereo_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_SoundMenu_Z80_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_SoundMenu_YM2612_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_SoundMenu_YM2612_Improved_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_SoundMenu_DAC_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_SoundMenu_DAC_Improved_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_SoundMenu_PSG_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_SoundMenu_PSG_Improved_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_SoundMenu_CDDA_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_SoundMenu_WAVDump_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_SoundMenu_GYMDump_activate(GtkMenuItem *menuitem, gpointer user_data);

// Options menu
void on_OptionsMenu_GeneralOptions_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_OptionsMenu_Joypads_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_OptionsMenu_Directories_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_OptionsMenu_BIOSMiscFiles_activate(GtkMenuItem *menuitem, gpointer user_data);

// Help menu
void on_HelpMenu_About_activate(GtkMenuItem *menuitem, gpointer user_data);


#ifdef __cplusplus
}
#endif

#endif
