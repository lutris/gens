/***************************************************************************
 * MDP: Sonic Gens. (Window Code) (Win32)                                  *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * SGens Copyright (c) 2002 by LOst                                        *
 * MDP port Copyright (c) 2008-2009 by David Korth                         *
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

// Win32 includes.
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

// C includes.
#include <stdio.h>

#include "sgens_window.h"
#include "sgens_plugin.h"
#include "sgens.h"

// Win32-specific includes.
#include "sgens_dllmain.h"
#include "resource.h"

// SGens ROM type information and widget information.
#include "sgens_rom_type.h"
#include "sgens_widget_info.h"

// MDP includes.
#include "mdp/mdp_error.h"
#include "mdp/mdp_event.h"
#include "mdp/mdp_mem.h"

// libgsft includes.
#include "libgsft/gsft_win32.h"
#include "libgsft/gsft_win32_gdi.h"
#include "libgsft/gsft_szprintf.h"

// Window.
static HWND	sgens_window = NULL;

// Window class.
static WNDCLASS	sgens_wndclass;

// Window size.
#define SGENS_WINDOW_WIDTH  336
#define SGENS_WINDOW_HEIGHT 328

#define FRAME_WIDTH (SGENS_WINDOW_WIDTH-16)
#define FRAME_LEVEL_INFO_HEIGHT  184
#define FRAME_PLAYER_INFO_HEIGHT 72

// Table size defines.
#define WIDGET_DESC_WIDTH 76
#define WIDGET_INFO_WIDTH 64
#define WIDGET_INTRACOL_SPACING 4
#define WIDGET_COL_SPACING 16
#define WIDGET_ROW_HEIGHT 16

// Font.
static HFONT sgens_hFont = NULL;

// Widgets.
static HWND	lblLoadedGame;
static HWND	lblLevelInfo_Zone;
static HWND	lblLevelInfo_Act;

static HWND	lblLevelInfo_Desc[LEVEL_INFO_COUNT];
static HWND	lblLevelInfo[LEVEL_INFO_COUNT];

static HWND	lblPlayerInfo_Desc[PLAYER_INFO_COUNT];
static HWND	lblPlayerInfo[PLAYER_INFO_COUNT];

// Window procedure.
static LRESULT CALLBACK sgens_window_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// Widget creation functions.
static void	sgens_window_create_child_windows(HWND hWnd);
static void	sgens_window_create_level_info_frame(HWND container);
static void	sgens_window_create_player_info_frame(HWND container);


/**
 * sgens_window_show(): Show the VDP Layer Options window.
 * @param parent Parent window.
 */
void MDP_FNCALL sgens_window_show(void *parent)
{
	if (sgens_window)
	{
		// Sonic Gens window is already visible. Set focus.
		// TODO: Figure out how to do this.
		ShowWindow(sgens_window, SW_SHOW);
		return;
	}
	
	// If no HINSTANCE was specified, use the main executable's HINSTANCE.
	if (!sgens_hInstance)
		sgens_hInstance = GetModuleHandle(NULL);
	
	if (sgens_wndclass.lpfnWndProc != sgens_window_wndproc)
	{
		// Create the window class.
		sgens_wndclass.style = 0;
		sgens_wndclass.lpfnWndProc = sgens_window_wndproc;
		sgens_wndclass.cbClsExtra = 0;
		sgens_wndclass.cbWndExtra = 0;
		sgens_wndclass.hInstance = sgens_hInstance;
		sgens_wndclass.hIcon = LoadIcon(sgens_hInstance, MAKEINTRESOURCE(IDI_SGENS));
		sgens_wndclass.hCursor = NULL;
		sgens_wndclass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
		sgens_wndclass.lpszMenuName = NULL;
		sgens_wndclass.lpszClassName = TEXT("mdp_misc_sgens_window");
		
		RegisterClass(&sgens_wndclass);
	}
	
	// Create the font.
	sgens_hFont = gsft_win32_gdi_get_message_font();
	
	// Create the window.
	sgens_window = CreateWindow(TEXT("mdp_misc_sgens_window"), TEXT("Sonic Gens"),
				    WS_DLGFRAME | WS_POPUP | WS_SYSMENU | WS_CAPTION,
				    CW_USEDEFAULT, CW_USEDEFAULT,
				    SGENS_WINDOW_WIDTH, SGENS_WINDOW_HEIGHT,
				    (HWND)parent, NULL, sgens_hInstance, NULL);
	
	// Window adjustment.
	gsft_win32_set_actual_window_size(sgens_window, SGENS_WINDOW_WIDTH, SGENS_WINDOW_HEIGHT);
	gsft_win32_center_on_window(sgens_window, (HWND)parent);
	
	// Update the current ROM type and information display.
	sgens_window_update_rom_type();
	sgens_window_update();
	
	UpdateWindow(sgens_window);
	ShowWindow(sgens_window, TRUE);
	
	// Register the window with MDP Host Services.
	sgens_host_srv->window_register(&mdp, sgens_window);
}


/**
 * sgens_window_create_child_windows(): Create child windows.
 * @param hWnd HWND of the parent window.
 */
static void sgens_window_create_child_windows(HWND hWnd)
{
	// Create the label for the loaded game.
	lblLoadedGame = CreateWindow(WC_STATIC, NULL,
				     WS_CHILD | WS_VISIBLE | SS_CENTER,
				     8, 8, SGENS_WINDOW_WIDTH-16, 16,
				     hWnd, NULL, sgens_hInstance, NULL);
	SetWindowFont(lblLoadedGame, sgens_hFont, TRUE);
	
	// Create the "Level Information" frame.
	sgens_window_create_level_info_frame(hWnd);
	
	// Create the "Player Information" frame.
	sgens_window_create_player_info_frame(hWnd);
	
	// Create the dialog buttons.
	
	// Close button.
	HWND btnClose = CreateWindow(WC_BUTTON, TEXT("&Close"),
				     WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				     SGENS_WINDOW_WIDTH-8-75, SGENS_WINDOW_HEIGHT-8-24,
				     75, 23,
				     hWnd, (HMENU)IDCLOSE, sgens_hInstance, NULL);
	SetWindowFont(btnClose, sgens_hFont, TRUE);
}


/**
 * sgens_window_create_level_info_frame(): Create the "Level Information" frame.
 * @param container Container for the frame.
 */
static void sgens_window_create_level_info_frame(HWND container)
{
	// "Level Information" frame.
	HWND fraLevelInfo = CreateWindow(WC_BUTTON, TEXT("Level Information"),
					 WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
					 8, 8+16,
					 FRAME_WIDTH, FRAME_LEVEL_INFO_HEIGHT,
					 container, NULL, sgens_hInstance, NULL);
	SetWindowFont(fraLevelInfo, sgens_hFont, TRUE);
	
	// "Zone" information label.
	lblLevelInfo_Zone = CreateWindow(WC_STATIC, TEXT("Zone"),
					 WS_CHILD | WS_VISIBLE | SS_CENTER,
					 8, 16, SGENS_WINDOW_WIDTH-16-16, 16,
					 fraLevelInfo, NULL, sgens_hInstance, NULL);
	SetWindowFont(lblLevelInfo_Zone, sgens_hFont, TRUE);
	
	// "Act" information label.
	lblLevelInfo_Act = CreateWindow(WC_STATIC, TEXT("Act"),
					WS_CHILD | WS_VISIBLE | SS_CENTER,
					8, 16+16, SGENS_WINDOW_WIDTH-16-16, 16,
					fraLevelInfo, NULL, sgens_hInstance, NULL);
	SetWindowFont(lblLevelInfo_Act, sgens_hFont, TRUE);
	
	// Table for level information.
	unsigned int i;
	for (i = 0; i < LEVEL_INFO_COUNT; i++)
	{
		// Determine the column starting and ending positions.
		int widget_desc_left, widget_desc_width, widget_info_width;
		if (level_info[i].fill_all_cols)
		{
			widget_desc_left = 8;
			widget_desc_width = FRAME_WIDTH - 48;
			widget_info_width = FRAME_WIDTH - 8 -
					    widget_desc_left -
					    widget_desc_width -
					    WIDGET_INTRACOL_SPACING;
		}
		else
		{
			widget_desc_left = 8 + ((WIDGET_DESC_WIDTH +
					    WIDGET_INTRACOL_SPACING +
					    WIDGET_INFO_WIDTH +
					    WIDGET_COL_SPACING) * level_info[i].column);
			widget_desc_width = WIDGET_DESC_WIDTH;
			widget_info_width = WIDGET_INFO_WIDTH;
		}
		
		const int widget_top = (16*3)+(level_info[i].row * WIDGET_ROW_HEIGHT);
		
		// Description label.
		lblLevelInfo_Desc[i] = CreateWindow(WC_STATIC, level_info[i].description,
						    WS_CHILD | WS_VISIBLE | SS_LEFT,
						    widget_desc_left, widget_top,
						    widget_desc_width, WIDGET_ROW_HEIGHT,
						    fraLevelInfo, NULL, sgens_hInstance, NULL);
		SetWindowFont(lblLevelInfo_Desc[i], sgens_hFont, TRUE);
		
		// Information label.
		// TODO: Monospace font.
		lblLevelInfo[i] = CreateWindow(WC_STATIC, level_info[i].initial,
					       WS_CHILD | WS_VISIBLE | SS_RIGHT,
					       widget_desc_left+widget_desc_width+WIDGET_INTRACOL_SPACING, widget_top,
					       widget_info_width, WIDGET_ROW_HEIGHT,
					       fraLevelInfo, NULL, sgens_hInstance, NULL);
	}
}


/**
 * sgens_window_create_player_info_frame(): Create the "Player Information" frame.
 * @param container Container for the frame.
 */
static void sgens_window_create_player_info_frame(HWND container)
{
	// "Level Information" frame.
	HWND fraPlayerInfo = CreateWindow(WC_BUTTON, TEXT("Player Information"),
					  WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
					  8, 8+16+FRAME_LEVEL_INFO_HEIGHT+8,
					  FRAME_WIDTH, FRAME_PLAYER_INFO_HEIGHT,
					  container, NULL, sgens_hInstance, NULL);
	SetWindowFont(fraPlayerInfo, sgens_hFont, TRUE);
	
	// Table for player information.
	unsigned int i;
	for (i = 0; i < PLAYER_INFO_COUNT; i++)
	{
		// Determine the column starting and ending positions.
		int widget_desc_left, widget_desc_width, widget_info_width;
		if (player_info[i].fill_all_cols)
		{
			widget_desc_left = 8;
			widget_desc_width = FRAME_WIDTH - 48;
			widget_info_width = FRAME_WIDTH - 8 -
					    widget_desc_left -
					    widget_desc_width -
					    WIDGET_INTRACOL_SPACING;
		}
		else
		{
			widget_desc_left = 8 + ((WIDGET_DESC_WIDTH +
					    WIDGET_INTRACOL_SPACING +
					    WIDGET_INFO_WIDTH +
					    WIDGET_COL_SPACING) * player_info[i].column);
			widget_desc_width = WIDGET_DESC_WIDTH;
			widget_info_width = WIDGET_INFO_WIDTH;
		}
		
		const int widget_top = 16 + (player_info[i].row * WIDGET_ROW_HEIGHT);
		
		// Description label.
		lblPlayerInfo_Desc[i] = CreateWindow(WC_STATIC, player_info[i].description,
						    WS_CHILD | WS_VISIBLE | SS_LEFT,
						    widget_desc_left, widget_top,
						    widget_desc_width, WIDGET_ROW_HEIGHT,
						    fraPlayerInfo, NULL, sgens_hInstance, NULL);
		SetWindowFont(lblPlayerInfo_Desc[i], sgens_hFont, TRUE);
		
		// Information label.
		// TODO: Monospace font.
		lblPlayerInfo[i] = CreateWindow(WC_STATIC, player_info[i].initial,
						WS_CHILD | WS_VISIBLE | SS_RIGHT,
						widget_desc_left+widget_desc_width+WIDGET_INTRACOL_SPACING, widget_top,
						widget_info_width, WIDGET_ROW_HEIGHT,
						fraPlayerInfo, NULL, sgens_hInstance, NULL);
	}
}


/**
 * sgens_window_close(): Close the VDP Layer Options window.
 */
void MDP_FNCALL sgens_window_close(void)
{
	if (!sgens_window)
		return;
	
	// Unregister the window from MDP Host Services.
	sgens_host_srv->window_unregister(&mdp, sgens_window);
	
	// Destroy the window.
	DestroyWindow(sgens_window);
	sgens_window = NULL;
}


/**
 * sgens_window_wndproc(): Window procedure.
 * @param hWnd hWnd of the window.
 * @param message Window message.
 * @param wParam
 * @param lParam
 * @return
 */
static LRESULT CALLBACK sgens_window_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CREATE:
			sgens_window_create_child_windows(hWnd);
			break;
		
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDCANCEL:
				case IDCLOSE:
					sgens_window_close();
					break;
				default:
					// Unknown identifier.
					break;
			}
			break;
		
		case WM_DESTROY:
			if (hWnd != sgens_window)
				break;
			
			sgens_window = NULL;
			break;
	}
	
	return DefWindowProc(hWnd, message, wParam, lParam);
}


/**
 * sgens_window_update_rom_type(): Update the current ROM type.
 */
void MDP_FNCALL sgens_window_update_rom_type(void)
{
	if (!sgens_window)
		return;
	
	if (sgens_current_rom_type < SGENS_ROM_TYPE_NONE ||
	    sgens_current_rom_type >= SGENS_ROM_TYPE_MAX)
	{
		// Invalid ROM type. Assume SGENS_ROM_TYPE_UNSUPPORTED.
		sgens_current_rom_type = SGENS_ROM_TYPE_UNSUPPORTED;
	}
	
	// Set the "Loaded Game" label.
	Static_SetText(lblLoadedGame, sgens_ROM_type_name[sgens_current_rom_type]);
	
	// Reset the "Rings for Perfect Bonus" information label.
	Static_SetText(lblLevelInfo[LEVEL_INFO_RINGS_PERFECT], TEXT("0"));
	
	// Enable/Disable the "Rings for Perfect Bonus" labels, depending on ROM type.
	BOOL isS2 = (sgens_current_rom_type >= SGENS_ROM_TYPE_SONIC2_REV00 &&
		     sgens_current_rom_type <= SGENS_ROM_TYPE_SONIC2_REV02);
	
	Static_Enable(lblLevelInfo_Desc[LEVEL_INFO_RINGS_PERFECT], isS2);
	Static_Enable(lblLevelInfo[LEVEL_INFO_RINGS_PERFECT], isS2);
}


/**
 * sgens_window_update(): Update the information display.
 */
void MDP_FNCALL sgens_window_update(void)
{
	if (!sgens_window)
		return;
	
	if (sgens_current_rom_type <= SGENS_ROM_TYPE_UNSUPPORTED)
		return;
	
	// String buffer.
	char tmp[64];
	
	// Get the widget information.
	sgens_widget_info info;
	sgens_get_widget_info(&info);
	
	// Values common to all supported Sonic games.
	
	// Score.
	szprintf(tmp, sizeof(tmp), "%d", info.score);
	Static_SetText(lblLevelInfo[LEVEL_INFO_SCORE], tmp);
	
	// Time.
	szprintf(tmp, sizeof(tmp), "%02d:%02d:%02d", info.time.min, info.time.sec, info.time.frames);
	Static_SetText(lblLevelInfo[LEVEL_INFO_TIME], tmp);
	
	// Rings.
	szprintf(tmp, sizeof(tmp), "%d", info.rings);
	Static_SetText(lblLevelInfo[LEVEL_INFO_RINGS], tmp);
	
	// Lives.
	szprintf(tmp, sizeof(tmp), "%d", info.lives);
	Static_SetText(lblLevelInfo[LEVEL_INFO_LIVES], tmp);
	
	// Continues.
	szprintf(tmp, sizeof(tmp), "%d", info.continues);
	Static_SetText(lblLevelInfo[LEVEL_INFO_CONTINUES], tmp);
	
	// Rings remaining for Perfect Bonus.
	// This is only applicable for Sonic 2.
	if (sgens_current_rom_type >= SGENS_ROM_TYPE_SONIC2_REV00 &&
	    sgens_current_rom_type <= SGENS_ROM_TYPE_SONIC2_REV02)
	{
		szprintf(tmp, sizeof(tmp), "%d", info.rings_for_perfect_bonus);
		Static_SetText(lblLevelInfo[LEVEL_INFO_RINGS_PERFECT], tmp);
	}
	
	// Water status.
	szprintf(tmp, sizeof(tmp), "%s", (info.water_level != 0 ? "ON" : "OFF"));
	Static_SetText(lblLevelInfo[LEVEL_INFO_WATER_ENABLED], tmp);
	
	// Water level.
	szprintf(tmp, sizeof(tmp), "%04X", info.water_level);
	Static_SetText(lblLevelInfo[LEVEL_INFO_WATER_LEVEL], tmp);
	
	// Number of emeralds.
	szprintf(tmp, sizeof(tmp), "%d", info.emeralds);
	Static_SetText(lblLevelInfo[LEVEL_INFO_EMERALDS], tmp);
	
	// Camera X position.
	szprintf(tmp, sizeof(tmp), "%04X", info.camera_x);
	Static_SetText(lblLevelInfo[LEVEL_INFO_CAMERA_X], tmp);
	
	// Camera Y position.
	szprintf(tmp, sizeof(tmp), "%04X", info.camera_y);
	Static_SetText(lblLevelInfo[LEVEL_INFO_CAMERA_Y], tmp);
	
	// Player angle.
	szprintf(tmp, sizeof(tmp), "%0.02f" DEGREE_SYMBOL, info.player_angle);
	Static_SetText(lblPlayerInfo[PLAYER_INFO_ANGLE], tmp);
	
	// Player X position.
	szprintf(tmp, sizeof(tmp), "%04X", info.player_x);
	Static_SetText(lblPlayerInfo[PLAYER_INFO_X], tmp);
	
	// Player Y position.
	szprintf(tmp, sizeof(tmp), "%04X", info.player_y);
	Static_SetText(lblPlayerInfo[PLAYER_INFO_Y], tmp);
	
	// SGens window has been updated.
	return;
}
