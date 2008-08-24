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
#include "mem_m68k.h"
#include "g_sdlsound.h"
#include "ym2612.h"
#include "psg.h"
#include "pcm.h"
#include "pwm.h"
#include "cd_sys.h"
#include "gym.h"


/**
 * Sync_GensWindow(): Synchronize the GENS Main Window.
 */
void Sync_GensWindow(void)
{
	// Synchronize all menus.
	Sync_GensWindow_FileMenu();
	Sync_GensWindow_GraphicsMenu();
	Sync_GensWindow_CPUMenu();
	Sync_GensWindow_SoundMenu();
	Sync_GensWindow_OptionsMenu();
}


/**
 * Sync_GensWindow_FileMenu(): Synchronize the File Menu.
 */
void Sync_GensWindow_FileMenu(void)
{
	GtkWidget *MItem_SaveState;
	
	// Disable callbacks so nothing gets screwed up.
	do_callbacks = 0;
	
	// TODO: ROM History
	
	// Current savestate
	sprintf(Str_Tmp, "FileMenu_ChangeState_SubMenu_%d", Current_State);
	MItem_SaveState = lookup_widget(gens_window, Str_Tmp);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_SaveState), TRUE);
	
	// TODO: Disable Close ROM if no ROM is loaded.
	
	// Enable callbacks.
	do_callbacks = 1;
}


/**
 * Sync_GensWindow_GraphicsMenu(): Synchronize the Graphics menu.
 */
void Sync_GensWindow_GraphicsMenu(void)
{
	GtkWidget *MItem_VSync, *MItem_Stretch, *MItem_OpenGL, *MItem_SpriteLimit;
	GtkWidget *MItem_OpenGL_Resolution, *MItem_bpp, *MItem_Render, *MItem_FrameSkip;
	
	// Disable callbacks so nothing gets screwed up.
	do_callbacks = 0;
	
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
	
	// Enable callbacks.
	do_callbacks = 1;
}


/**
 * Sync_GensWindow_CPUMenu(): Synchronize the CPU menu.
 */
void Sync_GensWindow_CPUMenu(void)
{
	GtkWidget *MItem_Reset68K, *MItem_ResetM68K, *MItem_ResetS68K;
	GtkWidget *MItem_ResetMSH2, *MItem_ResetSSH2;
	
	// Disable callbacks so nothing gets screwed up.
	do_callbacks = 0;

	// TODO: Debug
	// TODO: Country
	
	// Hide and show appropriate RESET items.
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
	
	// Enable callbacks.
	do_callbacks = 1;
}


/**
 * Sync_GensWindow_SoundMenu(): Synchronize the Sound menu.
 */
void Sync_GensWindow_SoundMenu(void)
{
	GtkWidget *MItem_Enable, *MItem_Rate, *MItem_Stereo, *MItem_Z80;
	GtkWidget *MItem_YM2612, *MItem_YM2612_Improved;
	GtkWidget *MItem_DAC, *MItem_DAC_Improved;
	GtkWidget *MItem_PSG, *MItem_PSG_Improved;
	GtkWidget *MItem_PCM, *MItem_PWM, *MItem_CDDA;
	
	GtkWidget *MItem_GYMDump, *MItem_WAVDump;
	gchar *label; int allowGYMDump;
	
	// Disable callbacks so nothing gets screwed up.
	do_callbacks = 0;
	
	// Simple checkbox items
	MItem_Enable = lookup_widget(gens_window, "SoundMenu_Enable");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_Enable), Sound_Enable);
	MItem_Stereo = lookup_widget(gens_window, "SoundMenu_Stereo");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_Stereo), Sound_Stereo);
	MItem_Z80 = lookup_widget(gens_window, "SoundMenu_Z80");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_Z80), Z80_State & 1);
	MItem_YM2612 = lookup_widget(gens_window, "SoundMenu_YM2612");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_YM2612), YM2612_Enable);
	MItem_YM2612_Improved = lookup_widget(gens_window, "SoundMenu_YM2612_Improved");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_YM2612_Improved), YM2612_Improv);
	MItem_DAC = lookup_widget(gens_window, "SoundMenu_DAC");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_DAC), DAC_Enable);
	MItem_DAC_Improved = lookup_widget(gens_window, "SoundMenu_DAC_Improved");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_DAC_Improved), DAC_Improv);
	MItem_PSG = lookup_widget(gens_window, "SoundMenu_PSG");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_PSG), PSG_Enable);
	MItem_PSG_Improved = lookup_widget(gens_window, "SoundMenu_PSG_Improved");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_PSG_Improved), PSG_Improv);
	MItem_PCM = lookup_widget(gens_window, "SoundMenu_PCM");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_PCM), PCM_Enable);
	MItem_PWM = lookup_widget(gens_window, "SoundMenu_PWM");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_PWM), PWM_Enable);
	MItem_CDDA = lookup_widget(gens_window, "SoundMenu_CDDA");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_CDDA), CDDA_Enable);
	
	// Rate
	sprintf(Str_Tmp, "SoundMenu_Rate_SubMenu_%d", Sound_Rate);
	MItem_Rate = lookup_widget(gens_window, Str_Tmp);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_Rate), TRUE);
	
	// GYM dumping
	label = (GYM_Dumping ? "Stop GYM Dump" : "Start GYM Dump");
	MItem_GYMDump = lookup_widget(gens_window, "SoundMenu_GYMDump");
	gtk_label_set_text(GTK_LABEL(GTK_BIN(MItem_GYMDump)->child), label);
	
	// WAV dumping
	label = (WAV_Dumping ? "Stop WAV Dump" : "Start WAV Dump");
	MItem_WAVDump = lookup_widget(gens_window, "SoundMenu_WAVDump");
	gtk_label_set_text(GTK_LABEL(GTK_BIN(MItem_WAVDump)->child), label);
	
	// Enable or disable items, depending on the Enable state.
	gtk_widget_set_sensitive(MItem_Stereo, Sound_Enable);
	gtk_widget_set_sensitive(MItem_YM2612, Sound_Enable);
	gtk_widget_set_sensitive(MItem_YM2612_Improved, Sound_Enable);
	gtk_widget_set_sensitive(MItem_DAC, Sound_Enable);
	gtk_widget_set_sensitive(MItem_DAC_Improved, Sound_Enable);
	gtk_widget_set_sensitive(MItem_PSG, Sound_Enable);
	gtk_widget_set_sensitive(MItem_PSG_Improved, Sound_Enable);
	gtk_widget_set_sensitive(MItem_PCM, Sound_Enable);
	gtk_widget_set_sensitive(MItem_PWM, Sound_Enable);
	gtk_widget_set_sensitive(MItem_CDDA, Sound_Enable);
	gtk_widget_set_sensitive(MItem_GYMDump, Sound_Enable);
	gtk_widget_set_sensitive(MItem_WAVDump, Sound_Enable);
	
	// Enable or disable GYM/WAV dumping, depending on if a game is running or not.
	// Also, don't enable this if sound is disabled.
	allowGYMDump = (Genesis_Started || SegaCD_Started || _32X_Started) && Sound_Enable;
	gtk_widget_set_sensitive(MItem_GYMDump, allowGYMDump);
	gtk_widget_set_sensitive(MItem_WAVDump, allowGYMDump);
	
	// Enable callbacks.
	do_callbacks = 1;
}


/**
 * Sync_GensWindow_SoundMenu(): Synchronize the Options menu.
 */
void Sync_GensWindow_OptionsMenu(void)
{
	GtkWidget *SRAMSize;
	
	if (BRAM_Ex_State & 0x100)
	{
		// RAM cart selected.
		sprintf(Str_Tmp, "OptionsMenu_SegaCDSRAMSize_SubMenu_%d", BRAM_Ex_Size);
	}
	else
	{
		// No RAM cart selected.
		strcpy(Str_Tmp, "OptionsMenu_SegaCDSRAMSize_SubMenu_None");
	}
	SRAMSize = lookup_widget(gens_window, Str_Tmp);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(SRAMSize), TRUE);
}
