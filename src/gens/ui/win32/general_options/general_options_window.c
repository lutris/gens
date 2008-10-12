/***************************************************************************
 * Gens: (Win32) General Options Window.                                   *
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

#include "general_options_window.h"
#include "general_options_window_callbacks.h"
#include "gens/gens_window.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "emulator/g_main.hpp"

// Includes with directory defines.
// TODO: Consolidate these into Settings.PathNames
#include "util/file/save.hpp"
#include "util/gfx/scrshot.h"
#include "util/file/ggenie.h"
#include "util/file/rom.hpp"

// Gens Win32 resources
#include "ui/win32/resource.h"

// Win32 common controls
#include <commctrl.h>

static WNDCLASS WndClass;
HWND general_options_window = NULL;

// Message colors.
const COLORREF GO_MsgColors[4][2] =
{
	{RGB(0xFF, 0xFF, 0xFF), RGB(0x00, 0x00, 0x00)},
	{RGB(0x00, 0x00, 0xFF), RGB(0xFF, 0xFF, 0xFF)},
	{RGB(0x00, 0xFF, 0x00), RGB(0x00, 0x00, 0x00)},
	{RGB(0xFF, 0x00, 0x00), RGB(0x00, 0x00, 0x00)},
};
HBRUSH go_MsgColors_brushes[4][2];
HPEN go_MsgColors_pens[4][2];

// Intro effect colors.
const COLORREF GO_IntroEffectColors[8][2] =
{
	{RGB(0x00, 0x00, 0x00), RGB(0xFF, 0xFF, 0xFF)},
	{RGB(0x00, 0x00, 0xFF), RGB(0xFF, 0xFF, 0xFF)},
	{RGB(0x00, 0xFF, 0x00), RGB(0x00, 0x00, 0x00)},
	{RGB(0x00, 0xFF, 0xFF), RGB(0x00, 0x00, 0x00)},
	{RGB(0xFF, 0x00, 0x00), RGB(0x00, 0x00, 0x00)},
	{RGB(0xFF, 0x00, 0xFF), RGB(0x00, 0x00, 0x00)},
	{RGB(0xFF, 0xFF, 0x00), RGB(0x00, 0x00, 0x00)},
	{RGB(0xFF, 0xFF, 0xFF), RGB(0x00, 0x00, 0x00)},
};
HBRUSH go_IntroEffectColors_brushes[8][2];
HPEN go_IntroEffectColors_pens[8][2];

static const int frameWidth = 304;
static const int frameHeight = 64;

// Controls
HWND go_chkAutoFixChecksum;
HWND go_chkAutoPause;
HWND go_chkFastBlur;
HWND go_chkSegaCDLEDs;

// Index 0: FPS; Index 1: msg
HWND go_chkMsgEnable[2];
HWND go_chkMsgDoubleSized[2];
HWND go_chkMsgTransparency[2];
HWND go_stcMsgColor[2][4];
HWND go_stcIntroEffectColor[8];
unsigned short go_stcColor_State[3];

static void createMsgFrame(HWND hWnd, const int index,
			   const int frameLeft, const int frameTop,
			   const char* title);

#if 0
static void create_color_radio_buttons(const char* title,
				       const char* groupName, 
				       const char** colors,
				       const int num,
				       GtkWidget* container);
#endif


/**
 * create_general_options_window(): Create the General Options Window.
 * @return Directory Configuration Window.
 */
HWND create_general_options_window(void)
{
	if (general_options_window)
	{
		// General Options window is already created. Set focus.
		// TODO: Figure out how to do this.
		ShowWindow(general_options_window, 1);
		return NULL;
	}
	
	// Create the window class.
	WndClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = General_Options_Window_WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = ghInstance;
	WndClass.hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_SONIC_HEAD));
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = "Gens_General_Options";
	
	RegisterClass(&WndClass);
	
	// Create the window.
	general_options_window = CreateWindowEx(NULL, "Gens_General_Options", "General Options",
						(WS_POPUP | WS_SYSMENU | WS_CAPTION) & ~(WS_MINIMIZE),
						CW_USEDEFAULT, CW_USEDEFAULT,
						frameWidth + 16, (frameHeight * 5) + 16, NULL, NULL, ghInstance, NULL);
	
	// Set the actual window size.
	Win32_setActualWindowSize(general_options_window, frameWidth + 16, (frameHeight * 5) + 16);
	
	// Center the window on the Gens window.
	Win32_centerOnGensWindow(general_options_window);
	
	UpdateWindow(general_options_window);
	return general_options_window;
}

void General_Options_Window_CreateChildWindows(HWND hWnd)
{
	HWND grpBox, lblIntroEffectColor;
	
	const int frameLeft = 8;
	int frameTop = 8;
	unsigned short i, j;
	
	// Create message color brushes and pens.
	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 2; j++)
		{
			go_MsgColors_brushes[i][j] = CreateSolidBrush(GO_MsgColors[i][j]);
			go_MsgColors_pens[i][j] = CreatePen(PS_SOLID, 1, GO_MsgColors[i][j]);
		}
	}
	
	// Create intro effect color brushes and pens.
	for (i = 0; i < 8; i++)
	{
		for (j = 0; j < 2; j++)
		{
			go_IntroEffectColors_brushes[i][j] = CreateSolidBrush(GO_IntroEffectColors[i][j]);
			go_IntroEffectColors_pens[i][j] = CreatePen(PS_SOLID, 1, GO_IntroEffectColors[i][j]);
		}
	}
	
	// Initialize the radio button states.
	go_stcColor_State[0] = 0;
	go_stcColor_State[1] = 0;
	go_stcColor_State[2] = 0;
	
	// System frame
	grpBox = CreateWindow(WC_BUTTON, "System",
			      WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
			      frameLeft, frameTop, frameWidth, frameHeight,
			      hWnd, NULL, ghInstance, NULL);
	SendMessage(grpBox, WM_SETFONT, (WPARAM)fntMain, 1);
	
	// Auto Fix Checksum
	go_chkAutoFixChecksum = CreateWindow(WC_BUTTON, "Auto Fix Checksum",
					     WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
					     frameLeft+8, frameTop+16, 128, 20,
					     hWnd, NULL, ghInstance, NULL);
	SendMessage(go_chkAutoFixChecksum, WM_SETFONT, (WPARAM)fntMain, 1);
	
	// Auto Pause
	go_chkAutoPause = CreateWindow(WC_BUTTON, "Auto Pause",
				       WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
				       frameLeft+8+128+8+16, frameTop+16, 128, 20,
				       hWnd, NULL, ghInstance, NULL);
	SendMessage(go_chkAutoPause, WM_SETFONT, (WPARAM)fntMain, 1);
	
	// Fast Blur
	go_chkFastBlur = CreateWindow(WC_BUTTON, "Fast Blur",
				      WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
				      frameLeft+8, frameTop+16+20, 128, 20,
				      hWnd, NULL, ghInstance, NULL);
	SendMessage(go_chkFastBlur, WM_SETFONT, (WPARAM)fntMain, 1);
	
	// Show SegaCD LEDs
	go_chkSegaCDLEDs = CreateWindow(WC_BUTTON, "Show SegaCD LEDs",
				        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
				        frameLeft+8+128+8+16, frameTop+16+20, 128, 20,
				        hWnd, NULL, ghInstance, NULL);
	SendMessage(go_chkSegaCDLEDs, WM_SETFONT, (WPARAM)fntMain, 1);
	
	// FPS counter frame
	frameTop += frameHeight + 8;
	createMsgFrame(hWnd, 0, frameLeft, frameTop, "FPS counter");
	
	// Message frame
	frameTop += frameHeight + 8;
	createMsgFrame(hWnd, 1, frameLeft, frameTop, "Message");
	
	// Miscellaneous frame
	frameTop += frameHeight + 8;
	grpBox = CreateWindow(WC_BUTTON, "Miscellaneous",
			      WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
			      frameLeft, frameTop, frameWidth, (frameHeight * 3) / 4,
			      hWnd, NULL, ghInstance, NULL);
	SendMessage(grpBox, WM_SETFONT, (WPARAM)fntMain, 1);
	
	// Intro effect color label
	lblIntroEffectColor = CreateWindow(WC_STATIC, "Intro Effect Color:",
					   WS_CHILD | WS_VISIBLE | SS_LEFT,
					   frameLeft+8, frameTop+16+2, 92, 12,
					   hWnd, NULL, ghInstance, NULL);
	SendMessage(lblIntroEffectColor, WM_SETFONT, (WPARAM)fntMain, 1);
	
	// Radio buttons
	for (i = 0; i < 8; i++)
	{
		go_stcIntroEffectColor[i] = CreateWindow(
				WC_STATIC, "", WS_CHILD | WS_VISIBLE | SS_CENTER | SS_OWNERDRAW | SS_NOTIFY,
				frameLeft+8+92+8+(i*(16+8)), frameTop+16+2, 16, 16,
				hWnd, (HMENU)(0xA010 + i), ghInstance, NULL);
	}
#if 0
	// Cancel
	button_go_Cancel = gtk_button_new_from_stock("gtk-cancel");
	gtk_widget_set_name(button_go_Cancel, "button_go_Cancel");
	gtk_widget_show(button_go_Cancel);
	gtk_box_pack_start(GTK_BOX(hbutton_box_go_buttonRow), button_go_Cancel, FALSE, FALSE, 0);
	gtk_widget_add_accelerator(button_go_Cancel, "activate", accel_group,
				   GDK_Escape, (GdkModifierType)(0), (GtkAccelFlags)(0));
	AddButtonCallback_Clicked(button_go_Cancel, on_button_go_Cancel_clicked);
	GLADE_HOOKUP_OBJECT(general_options_window, button_go_Cancel, "button_go_Cancel");
	
	// Apply
	button_go_Apply = gtk_button_new_from_stock("gtk-apply");
	gtk_widget_set_name(button_go_Apply, "button_go_Apply");
	gtk_widget_show(button_go_Apply);
	gtk_box_pack_start(GTK_BOX(hbutton_box_go_buttonRow), button_go_Apply, FALSE, FALSE, 0);
	AddButtonCallback_Clicked(button_go_Apply, on_button_go_Apply_clicked);
	GLADE_HOOKUP_OBJECT(general_options_window, button_go_Apply, "button_go_Apply");
	
	// Save
	button_go_Save = gtk_button_new_from_stock("gtk-save");
	gtk_widget_set_name(button_go_Save, "button_go_Save");
	gtk_widget_show(button_go_Save);
	gtk_box_pack_start(GTK_BOX(hbutton_box_go_buttonRow), button_go_Save, FALSE, FALSE, 0);
	AddButtonCallback_Clicked(button_go_Save, on_button_go_Save_clicked);
	gtk_widget_add_accelerator(button_go_Save, "activate", accel_group,
				   GDK_Return, (GdkModifierType)(0), (GtkAccelFlags)(0));
	gtk_widget_add_accelerator(button_go_Save, "activate", accel_group,
				   GDK_KP_Enter, (GdkModifierType)(0), (GtkAccelFlags)(0));
	GLADE_HOOKUP_OBJECT(general_options_window, button_go_Save, "button_go_Save");
#endif
}


static void createMsgFrame(HWND hWnd, const int index,
			   const int frameLeft, const int frameTop,
			   const char* title)
{
	// Message frame
	HWND grpBox, lblColor;
	int i;
	
	grpBox = CreateWindow(WC_BUTTON, title,
			      WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
			      frameLeft, frameTop, frameWidth, frameHeight,
			      hWnd, NULL, ghInstance, NULL);
	SendMessage(grpBox, WM_SETFONT, (WPARAM)fntMain, 1);
	
	// Enable
	go_chkMsgEnable[index] = CreateWindow(WC_BUTTON, "Enable",
					      WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
					      frameLeft+8, frameTop+16, 128, 20,
					      hWnd, NULL, ghInstance, NULL);
	SendMessage(go_chkMsgEnable[index], WM_SETFONT, (WPARAM)fntMain, 1);
	
	// Double Sized
	go_chkMsgDoubleSized[index] = CreateWindow(WC_BUTTON, "Double Sized",
						   WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
						   frameLeft+8+128+8+16, frameTop+16, 128, 20,
						   hWnd, NULL, ghInstance, NULL);
	SendMessage(go_chkMsgDoubleSized[index], WM_SETFONT, (WPARAM)fntMain, 1);
	
	// Transparency
	go_chkMsgTransparency[index] = CreateWindow(WC_BUTTON, "Transparency",
						    WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
						    frameLeft+8, frameTop+16+20, 128, 20,
						    hWnd, NULL, ghInstance, NULL);
	SendMessage(go_chkMsgTransparency[index], WM_SETFONT, (WPARAM)fntMain, 1);
	
	// Color label
	lblColor = CreateWindow(WC_STATIC, "Color:",
				WS_CHILD | WS_VISIBLE | SS_LEFT,
				frameLeft+8+128+8+16, frameTop+16+20+2, 32, 12,
				hWnd, NULL, ghInstance, NULL);
	SendMessage(lblColor, WM_SETFONT, (WPARAM)fntMain, 1);
	
	// Radio buttons
	for (i = 0; i < 4; i++)
	{
		go_stcMsgColor[index][i] = CreateWindow(
				WC_STATIC, "", WS_CHILD | WS_VISIBLE | SS_CENTER | SS_OWNERDRAW | SS_NOTIFY,
				frameLeft+8+128+8+32+4+16+(i*(16+8)), frameTop+16+20+2, 16, 16,
				hWnd, (HMENU)(0xA000 + ((index * 4) + i)), ghInstance, NULL);
	}
}
