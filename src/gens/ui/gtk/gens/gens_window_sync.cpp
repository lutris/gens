/***************************************************************************
 * Gens: (GTK+) Main Window - Synchronization Functions.                   *
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

// Message logging.
#include "macros/log_msg.h"

// C includes.
#include <string.h>

// C++ includes
#include <string>
#include <list>
#include <deque>
using std::string;
using std::list;
using std::deque;

// libgsft includes.
#include "libgsft/gsft_szprintf.h"

// GTK+ includes.
#include <gtk/gtk.h>
#include "gtk-misc.h"

#include "gens_window.h"
#include "gens_window_sync.hpp"
#include "gens_menu.hpp"

// Common UI functions.
#include "ui/common/gens/gens_menu.h"
#include "ui/common/gens/gens_window_sync.h"

#include "emulator/g_main.hpp"
#include "emulator/options.hpp"
#include "segacd/cd_sys.hpp"
#include "util/file/rom.hpp"
#include "debugger/debugger.hpp"

#include "util/sound/wave.h"
#include "util/sound/gym.hpp"

#include "gens_core/vdp/vdp_io.h"
#include "gens_core/vdp/vdp_rend.h"

#include "gens_core/mem/mem_m68k.h"
#include "gens_core/mem/mem_m68k_cd.h"

#include "gens_core/sound/ym2612.hpp"
#include "gens_core/sound/psg.h"
#include "gens_core/sound/pcm.h"
#include "gens_core/sound/pwm.h"

// Renderer / Blitter selection stuff.
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/misc/cpuflags.h"

// Plugin Manager and Render Manager.
#include "plugins/pluginmgr.hpp"
#include "plugins/rendermgr.hpp"

// File management functions.
#include "util/file/file.hpp"

// Video, Audio.
#include "video/vdraw.h"
#include "audio/audio.h"


// Internal functions.
static void Sync_Gens_Window_GraphicsMenu_Backend(GtkWidget *container);
static void Sync_Gens_Window_GraphicsMenu_Render(GtkWidget *container);
#ifdef GENS_DEBUGGER
static void Sync_Gens_Window_CPUMenu_Debug(GtkWidget *container);
#endif /* GENS_DEBUGGER */


/**
 * Sync_Gens_Window(): Synchronize the Gens Main Window.
 */
void Sync_Gens_Window(void)
{
	// Synchronize all menus.
	Sync_Gens_Window_FileMenu();
	Sync_Gens_Window_FileMenu_ROMHistory();
	Sync_Gens_Window_GraphicsMenu();
	Sync_Gens_Window_CPUMenu();
	Sync_Gens_Window_SoundMenu();
	Sync_Gens_Window_OptionsMenu();
	Sync_Gens_Window_PluginsMenu();
}


/**
 * Sync_Gens_Window_FileMenu(): Synchronize the File Menu.
 * This does NOT synchronize the ROM History submenu.
 */
void Sync_Gens_Window_FileMenu(void)
{
	// Disable callbacks so nothing gets screwed up.
	gens_menu_do_callbacks = 0;
	
	// Netplay is currently not usable.
	GtkWidget *mnuNetplay = gens_menu_find_item(IDM_FILE_NETPLAY);
	gtk_widget_set_sensitive(mnuNetplay, FALSE);
	
	// Disable "Close ROM" if no ROM is loaded.
	gtk_widget_set_sensitive(gens_menu_find_item(IDM_FILE_CLOSEROM), (Game != NULL));
	
	// Savestate menu items
	gboolean saveStateEnable = (Game != NULL);
	gtk_widget_set_sensitive(gens_menu_find_item(IDM_FILE_LOADSTATE), saveStateEnable);
	gtk_widget_set_sensitive(gens_menu_find_item(IDM_FILE_SAVESTATE), saveStateEnable);
	gtk_widget_set_sensitive(gens_menu_find_item(IDM_FILE_QUICKLOAD), saveStateEnable);
	gtk_widget_set_sensitive(gens_menu_find_item(IDM_FILE_QUICKSAVE), saveStateEnable);
	
	// Current savestate
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gens_menu_find_item(IDM_FILE_CHANGESTATE_0 + Current_State)), TRUE);
	
	// Enable callbacks.
	gens_menu_do_callbacks = 1;
}


/**
 * Sync_Gens_Window_FileMenu_ROMHistory(): Synchronize the File, ROM History submenu.
 */
void Sync_Gens_Window_FileMenu_ROMHistory(void)
{
	// Disable callbacks so nothing gets screwed up.
	gens_menu_do_callbacks = 0;
	
	// ROM History
	GtkWidget *mnuROMHistory = gens_menu_find_item(IDM_FILE_ROMHISTORY);
	
	// Check if the ROM History submenu already exists.
	GtkWidget *mnuROMHistory_sub = gtk_menu_item_get_submenu(GTK_MENU_ITEM(mnuROMHistory));
	if (mnuROMHistory_sub)
	{
		// Submenu currently exists. Delete it.
		gtk_widget_destroy(mnuROMHistory_sub);
	}
	
	// Create a new submenu.
	mnuROMHistory_sub = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(mnuROMHistory), mnuROMHistory_sub);
	
	GtkWidget *mnuROMHistory_item;
	string sROMHistoryEntry;
	unsigned int romFormat;
	int romNum = 0;
	
	for (deque<ROM::Recent_ROM_t>::iterator rom = ROM::Recent_ROMs.begin();
	     rom != ROM::Recent_ROMs.end(); rom++)
	{
		// Determine the ROM format.
		romFormat = ((*rom).type & ROMTYPE_SYS_MASK);
		if (romFormat >= ROMTYPE_SYS_MD && romFormat <= ROMTYPE_SYS_MCD32X)
			sROMHistoryEntry = gws_rom_format_prefix[romFormat];
		else
			sROMHistoryEntry = gws_rom_format_prefix[0];
		
		// Add a tab, a dash, and a space.
		sROMHistoryEntry += "\t- ";
		
		// Get the ROM filename.
		sROMHistoryEntry += File::GetNameFromPath((*rom).filename);
		
		if (!(*rom).z_filename.empty())
		{
			// ROM has a compressed filename.
			sROMHistoryEntry += " [" + (*rom).z_filename + "]";
		}
		
		// Add the ROM item to the ROM History submenu.
		mnuROMHistory_item = gtk_menu_item_new_with_label(sROMHistoryEntry.c_str());
		gtk_widget_show(mnuROMHistory_item);
		gtk_container_add(GTK_CONTAINER(mnuROMHistory_sub), mnuROMHistory_item);
		
		// Connect the signal.
		g_signal_connect((gpointer)mnuROMHistory_item, "activate",
				  G_CALLBACK(gens_gtk_menu_callback),
				  GINT_TO_POINTER(IDM_FILE_ROMHISTORY_1 + romNum));
		
		// Increment the ROM number.
		romNum++;
	}
	
	// If no recent ROMs were found, disable the ROM History submenu.
	gtk_widget_set_sensitive(mnuROMHistory, romNum);
	
	// Enable callbacks.
	gens_menu_do_callbacks = 1;
}


/**
 * Sync_Gens_Window_GraphicsMenu(): Synchronize the Graphics menu.
 */
void Sync_Gens_Window_GraphicsMenu(void)
{
	uint16_t id;
	GtkWidget *mnuItem;
	
	// Disable callbacks so nothing gets screwed up.
	gens_menu_do_callbacks = 0;
	
	// Enable Full Screen only if the current backend supports it.
	mnuItem = gens_menu_find_item(IDM_GRAPHICS_FULLSCREEN);
	gtk_widget_set_sensitive(mnuItem, (vdraw_cur_backend_flags & VDRAW_BACKEND_FLAG_FULLSCREEN));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(mnuItem), vdraw_get_fullscreen());
	
	// Show Menu Bar.
	mnuItem = gens_menu_find_item(IDM_GRAPHICS_MENUBAR);
	gtk_widget_set_sensitive(mnuItem, !vdraw_get_fullscreen());
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(mnuItem), Settings.showMenuBar);
	
	// Simple checkbox items
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gens_menu_find_item(IDM_GRAPHICS_VSYNC)), Video.VSync_W);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gens_menu_find_item(IDM_GRAPHICS_SPRITELIMIT)), Sprite_Over);
	
	// Stretch mode
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gens_menu_find_item(IDM_GRAPHICS_STRETCH_NONE + Options::stretch())), TRUE);
	
	// Enable VSync/Stretch only if the current backend supports it.
	gtk_widget_set_sensitive(gens_menu_find_item(IDM_GRAPHICS_VSYNC),
				 (vdraw_cur_backend_flags & VDRAW_BACKEND_FLAG_VSYNC));
	gtk_widget_set_sensitive(gens_menu_find_item(IDM_GRAPHICS_STRETCH),
				 (vdraw_cur_backend_flags & VDRAW_BACKEND_FLAG_STRETCH));
	
	// SDL color depth.
	switch (bppOut)
	{
		case 15:
			id = IDM_GRAPHICS_BPP_15;
			break;
		case 16:
			id = IDM_GRAPHICS_BPP_16;
			break;
		case 32:
			id = IDM_GRAPHICS_BPP_32;
			break;
		default:
			id = 0;
			break;
	}
	
	if (id != 0)
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gens_menu_find_item(id)), true);
	
	// Rebuild the Render submenu.
	Sync_Gens_Window_GraphicsMenu_Render(gens_menu_find_item(IDM_GRAPHICS_RENDER));
	
	// Frame Skip
	id = (IDM_GRAPHICS_FRAMESKIP_AUTO + 1) + Frame_Skip;
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gens_menu_find_item(id)), TRUE);
	
	// Screenshot
	gtk_widget_set_sensitive(gens_menu_find_item(IDM_GRAPHICS_SCREENSHOT), (Game != NULL));
	
	// Backend
	Sync_Gens_Window_GraphicsMenu_Backend(gens_menu_find_item(IDM_GRAPHICS_BACKEND));
	
#ifdef GENS_OPENGL
	//gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gens_menu_find_item(IDM_GRAPHICS_OPENGL)), Video.OpenGL);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gens_menu_find_item(IDM_GRAPHICS_OPENGL_FILTER)), Video.GL.glLinearFilter);
	
	// OpenGL Resolution
	int resID = 0;
	uint32_t curRes = GENS_GWS_RES(Video.GL.width, Video.GL.height);
	while (gws_opengl_resolutions[resID][0] != 0)
	{
		if (gws_opengl_resolutions[resID][0] == curRes)
		{
			id = gws_opengl_resolutions[resID][1];
			break;
		}
		
		// Next resolution.
		resID++;
	}
	
	if (gws_opengl_resolutions[resID][0] == 0)
		id = IDM_GRAPHICS_OPENGL_RES_CUSTOM;
	
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gens_menu_find_item(id)), TRUE);
	
	// Set the text of the custom resolution item.
	GtkWidget *mnuGLResCustom = gens_menu_find_item(IDM_GRAPHICS_OPENGL_RES_CUSTOM);
	if (id == IDM_GRAPHICS_OPENGL_RES_CUSTOM)
	{
		// Custom resolution. Set the text.
		char sCustomRes[32];
		szprintf(sCustomRes, sizeof(sCustomRes), "Custom... (%dx%d)", Video.GL.width, Video.GL.height);
		gtk_label_set_text(GTK_LABEL(gtk_bin_get_child(GTK_BIN(mnuGLResCustom))), sCustomRes);
	}
	else
	{
		// Predefined resolution.
		gtk_label_set_text(GTK_LABEL(gtk_bin_get_child(GTK_BIN(mnuGLResCustom))), "Custom...");
	}
#endif
	
	// Enable callbacks.
	gens_menu_do_callbacks = 1;
}


/**
 * Sync_Gens_Window_GraphicsMenu_Backend(): Synchronize the Graphics, Backend submenu.
 * @param container Container for this submenu.
 */
static void Sync_Gens_Window_GraphicsMenu_Backend(GtkWidget *container)
{
	// Check if the Graphics submenu already exists.
	GtkWidget *mnuBackend_sub = gtk_menu_item_get_submenu(GTK_MENU_ITEM(container));
	if (mnuBackend_sub)
	{
		// Submenu currently exists. Delete it.
		gtk_widget_destroy(mnuBackend_sub);
	}
	
	// Create a new submenu.
	mnuBackend_sub = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(container), mnuBackend_sub);
	
	// Add the backends.
	int curBackend = 0;
	GSList *radioGroup = NULL;
	GtkWidget *mnuItem;
	
	while (vdraw_backends[curBackend])
	{
		mnuItem = gtk_radio_menu_item_new_with_label(radioGroup, vdraw_backends[curBackend]->name);
		radioGroup = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(mnuItem));
		gtk_widget_show(mnuItem);
		gtk_container_add(GTK_CONTAINER(mnuBackend_sub), mnuItem);
		
		// Check if this backend is currently active.
		if (vdraw_cur_backend_id == curBackend)
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(mnuItem), true);
		
		// Check if this backend is broken.
		gtk_widget_set_sensitive(mnuItem, !vdraw_backends_broken[curBackend]);
		
		// Connect the signal.
		g_signal_connect((gpointer)mnuItem, "activate",
				 G_CALLBACK(gens_gtk_menu_callback),
				 GINT_TO_POINTER(IDM_GRAPHICS_BACKEND + 1 + curBackend));
		
		// Next backend.
		curBackend++;
	}
}


/**
 * Sync_Gens_Window_GraphicsMenu_Render(): Synchronize the Graphics, Render submenu.
 * @param container Container for this submenu.
 */
static void Sync_Gens_Window_GraphicsMenu_Render(GtkWidget *container)
{
	GtkWidget *mnuItem;
	GSList *radioGroup = NULL;
	
	// Check if the Render submenu already exists.
	GtkWidget *mnuSubMenu = gtk_menu_item_get_submenu(GTK_MENU_ITEM(container));
	if (mnuSubMenu)
	{
		// Submenu currently exists. Delete it.
		gtk_widget_destroy(mnuSubMenu);
	}
	
	// Create a new submenu.
	mnuSubMenu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(container), mnuSubMenu);
	
	// Create the render entries.
	unsigned int i = IDM_GRAPHICS_RENDER_NORMAL;
	list<mdp_render_t*>::iterator& selMDP = (vdraw_get_fullscreen() ? rendMode_FS : rendMode_W);
	
	for (list<mdp_render_t*>::iterator curPlugin = RenderMgr::begin();
	     curPlugin != RenderMgr::end(); curPlugin++, i++)
	{
		// Delete the menu item from the map, if it exists.
		gens_menu_map.erase(i);
		
		mnuItem = gtk_radio_menu_item_new_with_mnemonic(radioGroup, (*curPlugin)->tag);
		radioGroup = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(mnuItem));
		gtk_widget_show(mnuItem);
		gtk_container_add(GTK_CONTAINER(mnuSubMenu), mnuItem);
		
		// Check if this render mode is selected.
		if (selMDP == curPlugin)
		{
			// Render mode is selected.
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(mnuItem), true);
		}
		
		// Connect the signal.
		g_signal_connect((gpointer)mnuItem, "activate",
				  G_CALLBACK(gens_gtk_menu_callback),
				  GINT_TO_POINTER(i));
		
		// Add the menu item to the map.
		gens_menu_map.insert(gensMenuMapItem_t(i, mnuItem));
		
		// Insert a separator between the built-in renderers
		// and the external renderers if external renderers exist.
		if (i == IDM_GRAPHICS_RENDER_NORMAL + 1)
		{
			if (RenderMgr::size() > 2)
			{
				mnuItem = gtk_separator_menu_item_new();
				gtk_widget_show(mnuItem);
				gtk_container_add(GTK_CONTAINER(mnuSubMenu), mnuItem);
			}
		}
	}
}


/**
 * Sync_Gens_Window_CPUMenu(): Synchronize the CPU menu.
 */
void Sync_Gens_Window_CPUMenu(void)
{
	// Disable callbacks so nothing gets screwed up.
	gens_menu_do_callbacks = 0;
	
#ifdef GENS_DEBUGGER
	// Debug
	Sync_Gens_Window_CPUMenu_Debug(gens_menu_find_item(IDM_CPU_DEBUG));
#endif /* GENS_DEBUGGER */
	
	// Country code
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gens_menu_find_item(IDM_CPU_COUNTRY_AUTO + Country + 1)), TRUE);
	
	// Hide and show the appropriate RESET items.
	GtkWidget *mnuReset68K  = gens_menu_find_item(IDM_CPU_RESET68K);
	GtkWidget *mnuResetM68K = gens_menu_find_item(IDM_CPU_RESETMAIN68K);
	GtkWidget *mnuResetS68K = gens_menu_find_item(IDM_CPU_RESETSUB68K);
	GtkWidget *mnuResetMSH2 = gens_menu_find_item(IDM_CPU_RESETMAINSH2);
	GtkWidget *mnuResetSSH2 = gens_menu_find_item(IDM_CPU_RESETSUBSH2);
	
	if (SegaCD_Started)
	{
		// SegaCD: Hide regular 68000; show Main 68000 and Sub 68000.
		gtk_widget_hide(mnuReset68K);
		gtk_widget_show(mnuResetM68K);
		gtk_widget_show(mnuResetS68K);
	}
	else
	{
		// No SegaCD: Show regular 68000; hide Main 68000 and Sub 68000;
		gtk_widget_show(mnuReset68K);
		gtk_widget_hide(mnuResetM68K);
		gtk_widget_hide(mnuResetS68K);
	}
	
	if (_32X_Started)
	{
		// 32X: Show Main SH2 and Sub SH2.
		gtk_widget_show(mnuResetMSH2);
		gtk_widget_show(mnuResetSSH2);
	}
	else
	{
		// 32X: Hide Main SH2 and Sub SH2.
		gtk_widget_hide(mnuResetMSH2);
		gtk_widget_hide(mnuResetSSH2);
	}
	
	// SegaCD Perfect Sync
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gens_menu_find_item(IDM_CPU_SEGACDPERFECTSYNC)), SegaCD_Accurate);
	
	// Enable callbacks.
	gens_menu_do_callbacks = 1;
}


#ifdef GENS_DEBUGGER
/**
 * Sync_Gens_Window_CPUMenu_Debug(): Synchronize the CPU, Debug submenu.
 * @param container Container for this submenu.
 */
static void Sync_Gens_Window_CPUMenu_Debug(GtkWidget *container)
{
	gtk_widget_set_sensitive(container, (Game != NULL));
	
	// Hide/Show debug entries depending on the active console.
	if (Game != NULL)
	{
		GtkWidget *mnuDebugItem;
		bool checkDebug;
		for (int i = 1; i <= 9; i++)
		{
			// TODO: Use debug constants instead?
			if (i >= 1 && i <= 3)
				checkDebug = 1;
			else if (i >= 4 && i <= 6)
				checkDebug = SegaCD_Started;
			else if (i >= 7 && i <= 9)
				checkDebug = _32X_Started;
			else
			{
				// Shouldn't happen...
				LOG_MSG(gens, LOG_MSG_LEVEL_ERROR,
					"ERROR: i == %d", i);
				checkDebug = 0;
			}
			
			mnuDebugItem = gens_menu_find_item(IDM_CPU_DEBUG + i);
			if (checkDebug)
				gtk_widget_show(mnuDebugItem);
			else
				gtk_widget_hide(mnuDebugItem);
			
			// Make sure the check state for this debug item is correct.
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(mnuDebugItem), ((int)debug_mode == i));
		}
		
		// Separators
		mnuDebugItem = gens_menu_find_item(IDM_CPU_DEBUG_SEGACD_SEPARATOR);
		if (SegaCD_Started)
			gtk_widget_show(mnuDebugItem);
		else
			gtk_widget_hide(mnuDebugItem);
		
		mnuDebugItem = gens_menu_find_item(IDM_CPU_DEBUG_32X_SEPARATOR);
		if (_32X_Started)
			gtk_widget_show(mnuDebugItem);
		else
			gtk_widget_hide(mnuDebugItem);
	}
}
#endif /* GENS_DEBUGGER */


/**
 * Sync_Gens_Window_SoundMenu(): Synchronize the Sound menu.
 */
void Sync_Gens_Window_SoundMenu(void)
{
	// Disable callbacks so nothing gets screwed up.
	gens_menu_do_callbacks = 0;
	
	// Get the Enabled flag for the other menu items.
	bool soundEnabled = audio_get_enabled();
	
	// Enabled
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gens_menu_find_item(IDM_SOUND_ENABLE)), soundEnabled);
	
	const uint16_t soundMenuItems[10][2] =
	{
		{IDM_SOUND_STEREO,		audio_get_stereo()},
		{IDM_SOUND_Z80,			Z80_State & Z80_STATE_ENABLED},
		{IDM_SOUND_YM2612,		YM2612_Enable},
		{IDM_SOUND_YM2612_IMPROVED,	YM2612_Improv},
		{IDM_SOUND_DAC,			DAC_Enable},
		{IDM_SOUND_PSG,			PSG_Enable},
		{IDM_SOUND_PSG_SINE,		PSG_Improv},
		{IDM_SOUND_PCM,			PCM_Enable},
		{IDM_SOUND_PWM,			PWM_Enable},
		{IDM_SOUND_CDDA,		CDDA_Enable},
	};
	
	GtkWidget *mnuItem;
	for (int i = 0; i < 10; i++)
	{
		mnuItem = gens_menu_find_item(soundMenuItems[i][0]);
		
		gtk_widget_set_sensitive(mnuItem, soundEnabled);
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(mnuItem), soundMenuItems[i][1]);
	}
	
	// Rate
	// TODO: Improve this by using a hash or something.
	uint16_t id;
	switch (audio_get_sound_rate())
	{
		case 11025:
			id = IDM_SOUND_RATE_11025;
			break;
		case 22050:
			id = IDM_SOUND_RATE_22050;
			break;
		case 44100:
			id = IDM_SOUND_RATE_44100;
			break;
		default:
			// Default to 22,050 Hz.
			id = IDM_SOUND_RATE_22050;
			break;
	}
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gens_menu_find_item(id)), TRUE);
	
	const char* sLabel;
	
	// WAV dumping
	sLabel = (WAV_Dumping ? "Stop WAV Dump" : "Start WAV Dump");
	GtkWidget *mnuWAVDump = gens_menu_find_item(IDM_SOUND_WAVDUMP);
	gtk_label_set_text(GTK_LABEL(gtk_bin_get_child(GTK_BIN(mnuWAVDump))), sLabel);
	
	// GYM dumping
	sLabel = (GYM_Dumping ? "Stop GYM Dump" : "Start GYM Dump");
	GtkWidget *mnuGYMDump = gens_menu_find_item(IDM_SOUND_GYMDUMP);
	gtk_label_set_text(GTK_LABEL(gtk_bin_get_child(GTK_BIN(mnuGYMDump))), sLabel);
	
	// Enable or disable GYM/WAV dumping, depending on if a game is running or not.
	// Also, don't enable this if sound is disabled.
	
	bool allowAudioDump = (Game != NULL) && audio_get_enabled();
	
	// WAV dumping.
	gtk_widget_set_sensitive(mnuWAVDump, allowAudioDump);
	
	// GYM dumping.
	gtk_widget_set_sensitive(mnuGYMDump, allowAudioDump);
	
	// Enable callbacks.
	gens_menu_do_callbacks = 1;
}


/**
 * Sync_Gens_Window_PluginsMenu(): Synchronize the Plugins menu.
 */
void Sync_Gens_Window_PluginsMenu(void)
{
	// Disable callbacks so nothing gets screwed up.
	gens_menu_do_callbacks = 0;
	
	// Get the Plugins menu.
	GtkWidget *mnuPlugins = gens_menu_find_item(IDM_PLUGINS_MENU);
	
	// Delete the Plugins submenu.
	GtkWidget *mnuPlugins_sub = gtk_menu_item_get_submenu(GTK_MENU_ITEM(mnuPlugins));
	if (mnuPlugins_sub)
	{
		// Submenu currently exists. Delete it.
		gtk_widget_destroy(mnuPlugins_sub);
	}
	
	// Create a new submenu.
	mnuPlugins_sub = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(mnuPlugins), mnuPlugins_sub);
	
	// Add the Plugin Manager menu item.
	GtkWidget *mnuItem = gtk_menu_item_new_with_mnemonic("Plugin Manager");
	gtk_widget_show(mnuItem);
	gtk_container_add(GTK_CONTAINER(mnuPlugins_sub), mnuItem);
	
	// Set the callback handler.
	g_signal_connect((gpointer)mnuItem, "activate",
			 G_CALLBACK(gens_gtk_menu_callback),
			 GINT_TO_POINTER(IDM_PLUGINS_MANAGER));
	
	if (PluginMgr::lstMenuItems.size() != 0)
	{
		// Plugin menu items were created.
		
		// Add the Plugin Manager separator.
		mnuItem = gtk_separator_menu_item_new();
		gtk_widget_show(mnuItem);
		gtk_container_add(GTK_CONTAINER(mnuPlugins_sub), mnuItem);
		
		// Create the plugin menu items.
		string sMenuText;
		size_t mnemonicPos;
		for (list<mdpMenuItem_t>::iterator curMenuItem = PluginMgr::lstMenuItems.begin();
		     curMenuItem != PluginMgr::lstMenuItems.end(); curMenuItem++)
		{
			sMenuText = (*curMenuItem).text;
			mnemonicPos = sMenuText.find('&');
			if (mnemonicPos != string::npos)
				sMenuText[mnemonicPos] = '_';
			
			mnuItem = gtk_check_menu_item_new_with_mnemonic(sMenuText.c_str());
			gtk_widget_show(mnuItem);
			gtk_container_add(GTK_CONTAINER(mnuPlugins_sub), mnuItem);
			
			// Set the menu item check state.
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(mnuItem), (*curMenuItem).checked);
			
			// Set the callback handler.
			g_signal_connect((gpointer)mnuItem, "activate",
					G_CALLBACK(gens_gtk_menu_callback),
					GINT_TO_POINTER((*curMenuItem).id));
		}
	}
	
	// Enable callbacks.
	gens_menu_do_callbacks = 1;
}


/**
 * Sync_Gens_Window_OptionsMenu(): Synchronize the Options menu.
 */
void Sync_Gens_Window_OptionsMenu(void)
{
	// Disable callbacks so nothing gets screwed up.
	gens_menu_do_callbacks = 0;
	
	// SRAM Enabled
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gens_menu_find_item(IDM_OPTIONS_SRAM_ENABLED)), SRAM_Enabled);
	
	// SegaCD SRAM Size
	int bramID = ((BRAM_Ex_State & 0x100) ? BRAM_Ex_Size : -1);
	GtkWidget *mnuBRAMSize = gens_menu_find_item(IDM_OPTIONS_SEGACDSRAMSIZE_NONE + (bramID + 1));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(mnuBRAMSize), TRUE);
	
	// Enable callbacks.
	gens_menu_do_callbacks = 1;
}
