/***************************************************************************
 * Gens: (Win32) Main Window - Callback Functions.                         *
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

#include "emulator/g_main.hpp"
#include "ui/gens_ui.hpp"

#include "gens_window_callbacks.hpp"
#include "gens_window_sync.hpp"
#include "emulator/g_md.hpp"
#include "emulator/options.hpp"

// Menus.
#include "gens_menu.hpp"

// libgsft includes.
#include "libgsft/gsft_unused.h"
#include "libgsft/gsft_file.h"

#include "util/sound/gym.hpp"
#include "util/file/rom.hpp"
#include "gens_core/vdp/vdp_io.h"

// 32X
#include "gens_core/cpu/sh2/sh2.h"

// CD-ROM drive access
#ifdef GENS_CDROM
#include "segacd/cd_aspi.hpp"
#endif /* GENS_CDROM */

// Debugger
#ifdef GENS_DEBUGGER
#include "debugger/debugger.hpp"
#endif /* GENS_DEBUGGER */

// Plugin Manager.
#include "plugins/pluginmgr.hpp"

// File management functions.
#include "util/file/file.hpp"

// Audio Handler.
#include "audio/audio.h"

// C++ includes
#include <list>
using std::list;

// For some reason, these aren't extern'd anywhere...
extern "C"
{
	void main68k_reset();
	void sub68k_reset();
}

// New menu handler.
#include "ui/common/gens/gens_menu.h"
#include "ui/common/gens/gens_menu_callbacks.hpp"

// Non-Menu Command Definitions
#include "gens_window_cmds.h"

// Video, Audio, Input.
#include "video/vdraw.h"
#include "video/vdraw_cpp.hpp"
#include "input/input.h"
#include "input/input_dinput.hpp"

// Win32 includes.
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <shellapi.h>

static bool paintsEnabled = true;

static void on_gens_window_close(void);
static void on_gens_window_NonMenuCmd(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static void showPopupMenu(HWND hWnd, bool adjustMousePointer);
static void dragDropFile(HDROP hDrop);


// TODO: If a radio menu item is selected but is already enabled, don't do anything.


/**
 * Gens_Window_WndProc(): The Gens window procedure.
 * @param hWnd hWnd of the object sending a message.
 * @param message Message being sent by the object.
 * @param wParam
 * @param lParam
 * @return
 */
LRESULT CALLBACK Gens_Window_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	RECT rectGensWindow;
	HMENU mnuCallback;
	bool state;
	
	switch(message)
	{
		case WM_CLOSE:
			on_gens_window_close();
			return 0;
		
		case WM_CREATE:
			Settings.Active = 1;
			break;
		
		case WM_DESTROY:
			// Delete the menu command accelerator table.
			if (hAccelTable_Menu)
			{
				DestroyAcceleratorTable(hAccelTable_Menu);
				hAccelTable_Menu = NULL;
			}
			break;
		
		case WM_EXITSIZEMOVE:
			if (vdraw_get_fullscreen())
				break;
			
			// Save the window coordinates.
			GetWindowRect(hWnd, &rectGensWindow);
			Window_Pos.x = rectGensWindow.left;
			Window_Pos.y = rectGensWindow.top;
			break;
		
		case WM_ACTIVATE:
			if (LOWORD(wParam) != WA_INACTIVE)
			{
				// Set the DirectInput cooperative level.
				input_set_cooperative_level(hWnd);
				
				// Initialize joysticks.
				input_dinput_init_joysticks(hWnd);
				
				// Auto Pause - reactivate the game.
				Settings.Active = 1;
			}
			else
			{
				// Auto Pause - deactivate the game.
				if (Auto_Pause && Settings.Active)
				{
					Settings.Active = 0;
					audio_clear_sound_buffer();
				}
			}
			
			break;
		
		case WM_PAINT:
			HDC hDC;
			PAINTSTRUCT ps;
			
			hDC = BeginPaint(hWnd, &ps);
			
			if (paintsEnabled)
			{
				if (vdraw_restore_primary)
					vdraw_restore_primary();
				vdraw_flip(0);
			}
			
			EndPaint(hWnd, &ps);
			break;
		
		case WM_RBUTTONDOWN:
			if (vdraw_get_fullscreen())
				showPopupMenu(hWnd, true);
			else if (!Settings.showMenuBar)
				showPopupMenu(hWnd, false);
			break;
		
		case WM_COMMAND:
			// Menu item.
			switch (LOWORD(wParam) & 0xF000)
			{
				case IDCMD_NONMENU_COMMANDS:
					on_gens_window_NonMenuCmd(hWnd, message, wParam, lParam);
					break;
				
				default:
					// Menu item selected.
					mnuCallback = gens_menu_find_item(LOWORD(wParam));
					state = (GetMenuState(mnuCallback, LOWORD(wParam), MF_BYCOMMAND) & MF_CHECKED);
					gens_common_menu_callback(LOWORD(wParam), state);
					break;
			}
			break;
		
#ifdef GENS_DEBUGGER
		case WM_KEYDOWN:
			if (IS_DEBUGGING())
			{
				// Get modifiers.
				int mod = 0;
				if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
					mod |= GENS_KMOD_CTRL;
				if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
					mod |= GENS_KMOD_SHIFT;
				// TODO: Other modifiers: Alt, Win/Meta, etc.
				
				Debug_Event(wParam, mod);
			}
			break;
#endif /* GENS_DEBUGGER */
		
		case WM_DROPFILES:
			// A file was dragged onto the Gens window.
			dragDropFile((HDROP)wParam);
			break;
	}
	
	return DefWindowProc(hWnd, message, wParam, lParam);
}


/**
 * Window is closed.
 */
static void on_gens_window_close(void)
{
	//Modif N - making sure sound doesn't stutter on exit
	if (audio_initialized)
		audio_clear_sound_buffer();
	
	close_gens();
}


/**
 * on_gens_window_NonMenuCmd(): Non-Menu Command has been activated.
 * @param hWnd hWnd of the object sending a message.
 * @param message Message being sent by the object.
 * @param wParam LOWORD(wParam) == Command.
 * @param lParam
 */
static void on_gens_window_NonMenuCmd(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	GSFT_UNUSED_PARAMETER(hWnd);
	GSFT_UNUSED_PARAMETER(message);
	GSFT_UNUSED_PARAMETER(lParam);
	
	// Force a wakeup.
	GensUI::wakeup();
	
	switch (LOWORD(wParam))
	{
		case IDCMD_ESC:
			if (Quick_Exit)
				close_gens();
			
#ifdef GENS_DEBUGGER
			if (IS_DEBUGGING())
			{
				Options::setDebugMode(DEBUG_NONE);
				Settings.Paused = 0;
				Sync_Gens_Window_CPUMenu();
			}
			else
#endif /* GENS_DEBUGGER */
			if (Settings.Paused)
			{
				Settings.Paused = 0;
			}
			else
			{
				Settings.Paused = 1;
				if (ice == 2)
					ice = 3;
				//Pause_Screen();
				audio_clear_sound_buffer();
			}
			break;
		
		case IDCMD_PAUSE:
			if (Settings.Paused)
			{
				Settings.Paused = 0;
			}
			else
			{
				Settings.Paused = 1;
				if (ice == 2)
					ice = 3;
				//Pause_Screen();
				audio_clear_sound_buffer();
			}
			break;
		
		case IDCMD_FRAMESKIP_AUTO:
			Options::setFrameSkip(-1);
			Sync_Gens_Window_GraphicsMenu();
			break;
		
		case IDCMD_FRAMESKIP_DEC:
			if (Options::frameSkip() == -1)
			{
				Options::setFrameSkip(0);
				Sync_Gens_Window_GraphicsMenu();
			}
			else if (Options::frameSkip() > 0)
			{
				Options::setFrameSkip(Options::frameSkip() - 1);
				Sync_Gens_Window_GraphicsMenu();
			}
			break;
		
		case IDCMD_FRAMESKIP_INC:
			if (Options::frameSkip() == -1)
			{
				Options::setFrameSkip(1);
				Sync_Gens_Window_GraphicsMenu();
			}
			else if (Options::frameSkip() < 8)
			{
				Options::setFrameSkip(Options::frameSkip() + 1);
				Sync_Gens_Window_GraphicsMenu();
			}
			break;
		
		case IDCMD_SAVESLOT_DEC:
			Options::setSaveSlot((Options::saveSlot() + 9) % 10);
			Sync_Gens_Window_FileMenu();
			break;
		
		case IDCMD_SAVESLOT_INC:
			Options::setSaveSlot((Options::saveSlot() + 1) % 10);
			Sync_Gens_Window_FileMenu();
			break;
		
		case IDCMD_SWBLIT:
			Options::setSwRender(!Options::swRender());
			Sync_Gens_Window_GraphicsMenu();
			break;
		
		case IDCMD_FASTBLUR:
			Options::setFastBlur(!Options::fastBlur());
			Sync_Gens_Window_GraphicsMenu();
			break;
		
		case IDCMD_STRETCH_MODE:
			Options::setStretch((Options::stretch() + 1) % 4);
			Sync_Gens_Window_GraphicsMenu();
			break;
		
		case IDCMD_YM2612_IMPROVED:
			Options::setSoundYM2612_Improved(!Options::soundYM2612_Improved());
			Sync_Gens_Window_SoundMenu();
			break;
		
		case IDCMD_PSG_IMPROVED:
			Options::setSoundPSG_Sine(!Options::soundPSG_Sine());
			Sync_Gens_Window_SoundMenu();
			break;
		
		case IDCMD_FPS:
			vdraw_set_fps_enabled(!vdraw_get_fps_enabled());
			break;
		
		case IDCMD_RENDERMODE_DEC:
			Options::rendererPrev();
			break;
		
		case IDCMD_RENDERMODE_INC:
			Options::rendererNext();
			break;
		
#ifdef GENS_CDROM
		case IDCMD_CHANGE_CD:
			if (SegaCD_Started)
				Change_CD();
			break;
#endif
		
		case IDCMD_VDRAW_BACKEND:
		{
			int curBackend = vdraw_cur_backend_id;
			curBackend++;
			if (curBackend >= VDRAW_BACKEND_MAX)
				curBackend = 0;
			Options::setBackend((VDRAW_BACKEND)curBackend);
			break;
		}
		
		case IDCMD_CONGRATULATIONS:
			// Congratulations!
			if (congratulations == 0)
				congratulations = 1;
			break;
		
		default:
		{
			unsigned int value = LOWORD(wParam) & 0x0F;
			
			switch (LOWORD(wParam) & 0xFF00)
			{
				case IDCMD_SAVESLOT_NUMERIC:
					// Change the save slot.
					if (value > 9)
						break;
					
					Options::setSaveSlot(value);
					Sync_Gens_Window_FileMenu();
					break;
				
				case IDCMD_ROMHISTORY_NUMERIC:
					// Load a ROM from the ROM History submenu.
					if (value == 0 || value > 9 || ROM::Recent_ROMs.size() < value)
						break;
					
					//if ((Check_If_Kaillera_Running())) return 0;
					if (audio_get_gym_playing())
						gym_play_stop();
					
					ROM::openROM(ROM::Recent_ROMs.at(value - 1).filename,
						     ROM::Recent_ROMs.at(value - 1).z_filename);
					
					Sync_Gens_Window();
					break;
			}
		}
	}
}


/**
 * showPopupMenu(): Show the Popup Menu.
 * @param hWnd Window handle.
 * @param adjustMousePointer If true, adjusts the mouse pointer.
 */
static void showPopupMenu(HWND hWnd, bool adjustMousePointer)
{
	// Clear the sound buffer to prevent stuttering.
	audio_clear_sound_buffer();
	
	if (adjustMousePointer)
	{
		// Show the mouse pointer.
		while (ShowCursor(false) >= 0) { }
		while (ShowCursor(true) < 0) { }
	}
	
	POINT pt;
	GetCursorPos(&pt);
	SendMessage(hWnd, WM_PAINT, 0, 0);
	
	// Disable painting while the popup menu is open.
	paintsEnabled = false;
	TrackPopupMenu(MainMenu, TPM_LEFTALIGN | TPM_TOPALIGN, pt.x, pt.y, NULL, hWnd, NULL);
	paintsEnabled = true;
	
	if (adjustMousePointer)
	{
		// Hide the mouse pointer.
		while (ShowCursor(true) < 0) { }
		while (ShowCursor(false) >= 0) { }
	}
}


/**
 * dragDropFile(): Called when a file is dragged onto the Gens window.
 */
static void dragDropFile(HDROP hDrop)
{
	TCHAR filename[GENS_PATH_MAX];
	unsigned int rval;
	
	rval = DragQueryFile(hDrop, 0, filename, sizeof(filename));
	
	if (rval > 0 && rval < GENS_PATH_MAX)
	{
		// Check that the file exists.
		if (gsft_file_exists(filename))
		{
			// File exists. Open it as a ROM image.
			ROM::openROM(filename);
			Sync_Gens_Window();
		}
	}
	
	DragFinish(hDrop);
}
