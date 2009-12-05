/***************************************************************************
 * Gens: (Win32) General Options Window.                                   *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2009 by David Korth                                  *
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

#include "genopt_window.hpp"
#include "gens/gens_window.h"

// C includes.
#include <stdio.h>
#include <string.h>
#include <unistd.h>

// Win32 includes.
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <tchar.h>
#include "ui/win32/fonts.h"
#include "ui/win32/resource.h"

// libgsft includes.
#include "libgsft/gsft_win32.h"

// Main settings.
#include "emulator/g_main.hpp"
#include "emulator/options.hpp"
#include "gens_ui.hpp"

// Video Drawing.
#include "video/vdraw.h"

// On-Screen Display colors.
static const COLORREF genopt_colors_OSD[4][2] =
{
	{RGB(0xFF, 0xFF, 0xFF), RGB(0x00, 0x00, 0x00)},
	{RGB(0x00, 0x00, 0xFF), RGB(0xFF, 0xFF, 0xFF)},
	{RGB(0x00, 0xFF, 0x00), RGB(0x00, 0x00, 0x00)},
	{RGB(0xFF, 0x00, 0x00), RGB(0x00, 0x00, 0x00)},
};

// Intro effect colors.
static const COLORREF genopt_colors_IntroEffect[8][2] =
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


// Window.
HWND genopt_window;

// Window class.
static WNDCLASS genopt_wndclass;

// Window size.
#define GENOPT_WINDOW_WIDTH  344
#define GENOPT_WINDOW_HEIGHT 300

#define GENOPT_FRAME_WIDTH  160
#define GENOPT_FRAME_HEIGHT 252

// Window procedure.
static LRESULT CALLBACK genopt_window_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// Widgets.
static HWND	btnOK, btnCancel, btnApply;

// Widgets: On-Screen Display.
static HWND	chkOSD_Enable[2];
static HWND	chkOSD_DoubleSized[2];
static HWND	chkOSD_Transparency[2];
static HWND	optOSD_Color[2][4];

// Widgets: Intro Effect.
static HWND	cboIntroEffect;
static HWND	optIntroEffectColor[8];
static int	state_optColor[3];

// Widgets: Miscellaneous settings.
static HWND	chkMisc_AutoFixChecksum;
static HWND	chkMisc_AutoPause;
static HWND	chkMisc_FastBlur;
static HWND	chkMisc_SegaCDLEDs;
static HWND	chkMisc_BorderColorEmulation;
static HWND	chkMisc_PauseTint;

// GDI objects.
static HBRUSH	brushOSD[4][2];
static HPEN	penOSD[4][2];
static HBRUSH	brushIntroEffect[8][2];
static HPEN	penIntroEffect[8][2];

// Widget creation functions.
static void	genopt_window_create_child_windows(HWND hWnd);
static void	genopt_window_create_osd_frame(HWND container, LPCTSTR title, const int index,
					       const int x, const int y, const int w, const int h);

// Configuration load/save functions.
static void	genopt_window_init(void);
static void	genopt_window_save(void);

// Callbacks.
static void	genopt_window_callback_drawMsgRadioButton(int identifier, LPDRAWITEMSTRUCT lpDrawItem);
static void	genopt_window_callback_selectRadioButton(int identifier);


/**
 * genopt_window_show(): Show the General Options window.
 */
void genopt_window_show(void)
{
	if (genopt_window)
	{
		// Controller Configuration window is already visible. Set focus.
		// TODO: Figure out how to do this.
		ShowWindow(genopt_window, SW_SHOW);
		return;
	}
	
	// Create the window class.
	if (genopt_wndclass.lpfnWndProc != genopt_window_wndproc)
	{
		// Create the window class.
		genopt_wndclass.style = 0;
		genopt_wndclass.lpfnWndProc = genopt_window_wndproc;
		genopt_wndclass.cbClsExtra = 0;
		genopt_wndclass.cbWndExtra = 0;
		genopt_wndclass.hInstance = ghInstance;
		genopt_wndclass.hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_GENS_APP));
		genopt_wndclass.hCursor = NULL;
		genopt_wndclass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
		genopt_wndclass.lpszMenuName = NULL;
		genopt_wndclass.lpszClassName = TEXT("genopt_window");
		
		RegisterClass(&genopt_wndclass);
	}
	
	// Create the window.
	genopt_window = CreateWindow(TEXT("genopt_window"), TEXT("General Options"),
				     WS_DLGFRAME | WS_POPUP | WS_SYSMENU | WS_CAPTION,
				     CW_USEDEFAULT, CW_USEDEFAULT,
				     GENOPT_WINDOW_WIDTH, GENOPT_WINDOW_HEIGHT,
				     gens_window, NULL, ghInstance, NULL);
	
	// Set the actual window size.
	gsft_win32_set_actual_window_size(genopt_window, GENOPT_WINDOW_WIDTH, GENOPT_WINDOW_HEIGHT);
	
	// Center the window on the Gens window.
	gsft_win32_center_on_window(genopt_window, gens_window);
	
	UpdateWindow(genopt_window);
	ShowWindow(genopt_window, SW_SHOW);
}


/**
 * genopt_window_create_child_windows(): Create child windows.
 * @param hWnd HWND of the parent window.
 */
static void genopt_window_create_child_windows(HWND hWnd)
{
	HWND grpBox;
	
	int frameLeft = 8;
	int frameTop = 8;
	
	// Create message color brushes and pens.
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			brushOSD[i][j] = CreateSolidBrush(genopt_colors_OSD[i][j]);
			penOSD[i][j] = CreatePen(PS_SOLID, 1, genopt_colors_OSD[i][j]);
		}
	}
	
	// Create intro effect color brushes and pens.
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			brushIntroEffect[i][j] = CreateSolidBrush(genopt_colors_IntroEffect[i][j]);
			penIntroEffect[i][j] = CreatePen(PS_SOLID, 1, genopt_colors_IntroEffect[i][j]);
		}
	}
	
	// Initialize the radio button states.
	state_optColor[0] = 0;
	state_optColor[1] = 0;
	state_optColor[2] = 0;
	
	// On-Screen Display frame
	HWND fraOSD = CreateWindow(WC_BUTTON, TEXT("On-Screen Display"),
				   WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
				   8, 8, GENOPT_FRAME_WIDTH, GENOPT_FRAME_HEIGHT,
				   hWnd, NULL, ghInstance, NULL);
	SetWindowFont(fraOSD, fntMain, true);
	
	// FPS counter frame
	genopt_window_create_osd_frame(hWnd, TEXT("FPS counter"), 0, 16, 24,
				       GENOPT_FRAME_WIDTH-16, (GENOPT_FRAME_HEIGHT/2)-16);
	
	// Message counter frame
	genopt_window_create_osd_frame(hWnd, TEXT("Message"), 1, 16, (GENOPT_FRAME_HEIGHT/2)+16,
				       GENOPT_FRAME_WIDTH-16, (GENOPT_FRAME_HEIGHT/2)-16);
	
	// Miscellaneous frame
	frameLeft += GENOPT_FRAME_WIDTH + 8;
	
	grpBox = CreateWindow(WC_BUTTON, TEXT("Miscellaneous"),
			      WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
			      frameLeft, frameTop, GENOPT_FRAME_WIDTH, GENOPT_FRAME_HEIGHT,
			      hWnd, NULL, ghInstance, NULL);
	SetWindowFont(grpBox, fntMain, true);
	
	// Auto Fix Checksum
	frameTop += 16;
	chkMisc_AutoFixChecksum = CreateWindow(WC_BUTTON, TEXT("Auto Fix Checksum"),
					       WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
					       frameLeft+8, frameTop, 128, 20,
					       hWnd, NULL, ghInstance, NULL);
	SetWindowFont(chkMisc_AutoFixChecksum, fntMain, true);
	
	// Auto Pause
	frameTop += 20;
	chkMisc_AutoPause = CreateWindow(WC_BUTTON, TEXT("Auto Pause"),
					 WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
					 frameLeft+8, frameTop, 128, 20,
					 hWnd, NULL, ghInstance, NULL);
	SetWindowFont(chkMisc_AutoPause, fntMain, true);
	
	// Fast Blur
	frameTop += 20;
	chkMisc_FastBlur = CreateWindow(WC_BUTTON, TEXT("Fast Blur"),
					WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
					frameLeft+8, frameTop, 128, 20,
					hWnd, NULL, ghInstance, NULL);
	SetWindowFont(chkMisc_FastBlur, fntMain, true);
	
	// Show SegaCD LEDs
	frameTop += 20;
	chkMisc_SegaCDLEDs = CreateWindow(WC_BUTTON, TEXT("Show SegaCD LEDs"),
					  WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
					  frameLeft+8, frameTop, 128, 20,
					  hWnd, NULL, ghInstance, NULL);
	SetWindowFont(chkMisc_SegaCDLEDs, fntMain, true);
	
	// Border Color Emulation
	frameTop += 20;
	chkMisc_BorderColorEmulation = CreateWindow(WC_BUTTON, TEXT("Border Color Emulation"),
						    WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
						    frameLeft+8, frameTop, 128, 20,
						    hWnd, NULL, ghInstance, NULL);
	SetWindowFont(chkMisc_BorderColorEmulation, fntMain, true);
	
	// Pause Tint
	frameTop += 20;
	chkMisc_PauseTint = CreateWindow(WC_BUTTON, TEXT("Pause Tint"),
						WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
						frameLeft+8, frameTop, 128, 20,
						hWnd, NULL, ghInstance, NULL);
	SetWindowFont(chkMisc_PauseTint, fntMain, true);
	
	// Intro Effect label.
	frameTop += 16+8;
	HWND lblIntroEffect = CreateWindow(WC_STATIC, TEXT("Intro Effect:"),
					   WS_CHILD | WS_VISIBLE | SS_CENTER,
					   frameLeft+8, frameTop, GENOPT_FRAME_WIDTH-16, 20,
					   hWnd, NULL, ghInstance, NULL);
	SetWindowFont(lblIntroEffect, fntMain, true);
	
	// Dropdown for intro effect.
	frameTop += 16;
	cboIntroEffect = CreateWindow(WC_COMBOBOX, NULL,
				      WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
				      frameLeft+8, frameTop, GENOPT_FRAME_WIDTH-16, 23*3,
				      hWnd, NULL, ghInstance, NULL);
	SetWindowFont(cboIntroEffect, fntMain, true);
	ComboBox_AddString(cboIntroEffect, TEXT("None"));
	ComboBox_AddString(cboIntroEffect, TEXT("Gens Logo Effect"));
	ComboBox_AddString(cboIntroEffect, TEXT("\"Crazy\" Effect"));
	//ComboBox_AddString(cboIntroEffect, TEXT("Genesis TMSS")); // TODO: Broken.
	
	// Intro effect color label
	frameTop += 24+4;
	HWND lblIntroEffectColor = CreateWindow(WC_STATIC, TEXT("Intro Effect Color:"),
						WS_CHILD | WS_VISIBLE | SS_CENTER,
						frameLeft+8, frameTop, GENOPT_FRAME_WIDTH-16, 20,
						hWnd, NULL, ghInstance, NULL);
	SetWindowFont(lblIntroEffectColor, fntMain, true);
	
	// Intro effect color buttons.
	frameLeft += 8+4 + (((GENOPT_FRAME_WIDTH-16) - (4*(16+8))) / 2);
	frameTop += 20;
	for (int i = 0; i < 8; i++)
	{
		optIntroEffectColor[i] = CreateWindow(
				WC_STATIC, NULL, WS_CHILD | WS_VISIBLE | SS_CENTER | SS_OWNERDRAW | SS_NOTIFY,
				frameLeft + ((i%4)*(16+8)), frameTop + ((i / 4) * (16+4)), 16, 16,
				hWnd, (HMENU)(0xA010 + i), ghInstance, NULL);
	}
	
	// Create the dialog buttons.
	
	// OK button.
	btnOK = CreateWindow(WC_BUTTON, TEXT("&OK"),
			     WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
			     GENOPT_WINDOW_WIDTH-8-75-8-75-8-75, GENOPT_WINDOW_HEIGHT-8-24,
			     75, 23,
			     hWnd, (HMENU)IDOK, ghInstance, NULL);
	SetWindowFont(btnOK, fntMain, true);
	
	// Cancel button.
	btnCancel = CreateWindow(WC_BUTTON, TEXT("&Cancel"),
				 WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				 GENOPT_WINDOW_WIDTH-8-75-8-75, GENOPT_WINDOW_HEIGHT-8-24,
				 75, 23,
				 hWnd, (HMENU)IDCANCEL, ghInstance, NULL);
	SetWindowFont(btnCancel, fntMain, true);
	
	// Apply button.
	btnApply = CreateWindow(WC_BUTTON, TEXT("&Apply"),
				WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				GENOPT_WINDOW_WIDTH-8-75, GENOPT_WINDOW_HEIGHT-8-24,
				75, 23,
				hWnd, (HMENU)IDAPPLY, ghInstance, NULL);
	SetWindowFont(btnApply, fntMain, true);
	
	// Initialize the internal data variables.
	genopt_window_init();
	
	// Set focus to "FPS counter" - "Enable".
	SetFocus(chkOSD_Enable[0]);
}


static void genopt_window_create_osd_frame(HWND container, LPCTSTR title, const int index,
					   const int x, const int y, const int w, const int h)
{
	// Message frame
	HWND grpBox, lblColor;
	
	grpBox = CreateWindow(WC_BUTTON, title,
			      WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_GROUPBOX,
			      x, y, w, h,
			      container, NULL, ghInstance, NULL);
	SetWindowFont(grpBox, fntMain, true);
	
	// Enable
	chkOSD_Enable[index] = CreateWindow(WC_BUTTON, TEXT("Enable"),
					    WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
					    x+8, y+16, 128, 20,
					    container, NULL, ghInstance, NULL);
	SetWindowFont(chkOSD_Enable[index], fntMain, true);
	
	// Double Sized
	chkOSD_DoubleSized[index] = CreateWindow(WC_BUTTON, TEXT("Double Sized"),
						 WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
						 x+8, y+16+20, 128, 20,
						 container, NULL, ghInstance, NULL);
	SetWindowFont(chkOSD_DoubleSized[index], fntMain, true);
	
	// Transparency
	chkOSD_Transparency[index] = CreateWindow(WC_BUTTON, TEXT("Transparency"),
						  WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
						  x+8, y+16+20+20, 128, 20,
						  container, NULL, ghInstance, NULL);
	SetWindowFont(chkOSD_Transparency[index], fntMain, true);
	
	// Color label
	lblColor = CreateWindow(WC_STATIC, TEXT("Color:"),
				WS_CHILD | WS_VISIBLE | SS_LEFT,
				x+8, y+16+20+20+20+2, 36, 20,
				container, NULL, ghInstance, NULL);
	SetWindowFont(lblColor, fntMain, true);
	
	// Radio buttons
	for (int i = 0; i < 4; i++)
	{
		optOSD_Color[index][i] = CreateWindow(
				WC_STATIC, NULL, WS_CHILD | WS_VISIBLE | SS_CENTER | SS_OWNERDRAW | SS_NOTIFY,
				x+8+36+4+(i*(16+8)), y+16+20+20+20+2, 16, 16,
				container, (HMENU)(0xA000 + ((index * 4) + i)), ghInstance, NULL);
	}
}


/**
 * genopt_window_close(): Close the General Options window.
 */
void genopt_window_close(void)
{
	if (!genopt_window)
		return;
	
	// Destroy the window.
	DestroyWindow(genopt_window);
	genopt_window = NULL;
	
	// Delete OSD color brushes and pens.
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			DeleteBrush(brushOSD[i][j]);
			DeletePen(penOSD[i][j]);
		}
	}
	
	// Delete intro effect color brushes and pens.
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			DeleteBrush(brushIntroEffect[i][j]);
			DeletePen(penIntroEffect[i][j]);
		}
	}
}


/**
 * genopt_window_init(): Initialize the internal variables.
 */
static void genopt_window_init(void)
{
	// Get the current options.
	unsigned char curFPSStyle, curMsgStyle;
	
	// Miscellaneous
	Button_SetCheck(chkMisc_AutoFixChecksum, (Auto_Fix_CS ? BST_CHECKED : BST_UNCHECKED));
	Button_SetCheck(chkMisc_AutoPause, (Auto_Pause ? BST_CHECKED : BST_UNCHECKED));
	Button_SetCheck(chkMisc_FastBlur, (vdraw_get_fast_blur() ? BST_CHECKED : BST_UNCHECKED));
	Button_SetCheck(chkMisc_SegaCDLEDs, (Show_LED ? BST_CHECKED : BST_UNCHECKED));
	Button_SetCheck(chkMisc_BorderColorEmulation, (Video.borderColorEmulation ? BST_CHECKED : BST_UNCHECKED));
	Button_SetCheck(chkMisc_PauseTint, (Video.pauseTint ? BST_CHECKED : BST_UNCHECKED));
	
	// FPS counter
	Button_SetCheck(chkOSD_Enable[0], (vdraw_get_fps_enabled() ? BST_CHECKED : BST_UNCHECKED));
	
	curFPSStyle = vdraw_get_fps_style();
	Button_SetCheck(chkOSD_DoubleSized[0], ((curFPSStyle & 0x10) ? BST_CHECKED : BST_UNCHECKED));
	Button_SetCheck(chkOSD_Transparency[0], ((curFPSStyle & 0x08) ? BST_CHECKED : BST_UNCHECKED));
	state_optColor[0] = (curFPSStyle & 0x06) >> 1;
	
	// Message
	Button_SetCheck(chkOSD_Enable[1], (vdraw_get_msg_enabled() ? BST_CHECKED : BST_UNCHECKED));
	
	curMsgStyle = vdraw_get_msg_style();
	Button_SetCheck(chkOSD_DoubleSized[1], ((curMsgStyle & 0x10) ? BST_CHECKED : BST_UNCHECKED));
	Button_SetCheck(chkOSD_Transparency[1], ((curMsgStyle & 0x08) ? BST_CHECKED : BST_UNCHECKED));
	state_optColor[1] = (curMsgStyle & 0x06) >> 1;
	
	// Intro effect.
	ComboBox_SetCurSel(cboIntroEffect, Intro_Style);
	if (ComboBox_GetCurSel(cboIntroEffect) == -1)
		ComboBox_SetCurSel(cboIntroEffect, 0);
	state_optColor[2] = vdraw_get_intro_effect_color();
}


/**
 * genopt_window_save(): Save the settings.
 */
static void genopt_window_save(void)
{
	// Save the current options.
	unsigned char curFPSStyle, curMsgStyle;
	
	// System
	Auto_Fix_CS = (Button_GetCheck(chkMisc_AutoFixChecksum) == BST_CHECKED);
	Auto_Pause = (Button_GetCheck(chkMisc_AutoPause) == BST_CHECKED);
	vdraw_set_fast_blur(Button_GetCheck(chkMisc_FastBlur) == BST_CHECKED);
	Show_LED = (Button_GetCheck(chkMisc_SegaCDLEDs) == BST_CHECKED);
	Video.borderColorEmulation = (Button_GetCheck(chkMisc_BorderColorEmulation) == BST_CHECKED);
	Video.pauseTint = (Button_GetCheck(chkMisc_PauseTint) == BST_CHECKED);
	
	// If Auto Pause is enabled, deactivate emulation.
	// If Auto Pause is disabled, activate emulation.
	Settings.Active = !Auto_Pause;
	
	// FPS counter
	vdraw_set_fps_enabled(Button_GetCheck(chkOSD_Enable[0]) == BST_CHECKED);
	
	curFPSStyle = vdraw_get_fps_style() & ~0x18;
	curFPSStyle |= ((Button_GetCheck(chkOSD_DoubleSized[0]) == BST_CHECKED) ? 0x10 : 0x00);
	curFPSStyle |= ((Button_GetCheck(chkOSD_Transparency[0]) == BST_CHECKED) ? 0x08 : 0x00);
	curFPSStyle &= ~0x06;
	curFPSStyle |= state_optColor[0] << 1;
	vdraw_set_fps_style(curFPSStyle);
	
	// Message
	vdraw_set_msg_enabled(Button_GetCheck(chkOSD_Enable[1]) == BST_CHECKED);
	curMsgStyle = vdraw_get_msg_style() & ~0x18;
	curMsgStyle |= ((Button_GetCheck(chkOSD_DoubleSized[1]) == BST_CHECKED) ? 0x10 : 0x00);
	curMsgStyle |= ((Button_GetCheck(chkOSD_Transparency[1]) == BST_CHECKED) ? 0x08 : 0x00);
	curMsgStyle &= ~0x06;
	curMsgStyle |= state_optColor[1] << 1;
	vdraw_set_msg_style(curMsgStyle);
	
	// Intro effect.
	Intro_Style = ComboBox_GetCurSel(cboIntroEffect);
	if (Intro_Style < 0)
		Intro_Style = 0;
	vdraw_set_intro_effect_color(static_cast<unsigned char>(state_optColor[2]));
}


/**
 * genopt_window_wndproc(): Window procedure.
 * @param hWnd hWnd of the window.
 * @param message Window message.
 * @param wParam
 * @param lParam
 * @return
 */
static LRESULT CALLBACK genopt_window_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CREATE:
			genopt_window_create_child_windows(hWnd);
			break;
		
		case WM_DRAWITEM:
			genopt_window_callback_drawMsgRadioButton(LOWORD(wParam), (LPDRAWITEMSTRUCT)lParam);
			break;
		
		case WM_COMMAND:
			// Button press
			switch (LOWORD(wParam))
			{
				case IDOK:
					genopt_window_save();
					DestroyWindow(hWnd);
					break;
				
				case IDAPPLY:
					genopt_window_save();
					break;
				
				case IDCANCEL:
					DestroyWindow(hWnd);
					break;
				
				default:
					if ((LOWORD(wParam) & 0xFF00) == 0xA000)
					{
						// Radio button selected.
						genopt_window_callback_selectRadioButton(LOWORD(wParam));
					}
			}
			break;
		
		case WM_DESTROY:
			if (hWnd != genopt_window)
				break;
			
			genopt_window = NULL;
			
			// Delete OSD color brushes and pens.
			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 2; j++)
				{
					DeleteBrush(brushOSD[i][j]);
					DeletePen(penOSD[i][j]);
				}
			}
			
			// Delete intro effect color brushes and pens.
			for (int i = 0; i < 8; i++)
			{
				for (int j = 0; j < 2; j++)
				{
					DeleteBrush(brushIntroEffect[i][j]);
					DeletePen(penIntroEffect[i][j]);
				}
			}
			
			break;
	}
	
	return DefWindowProc(hWnd, message, wParam, lParam);
}


static void genopt_window_callback_drawMsgRadioButton(int identifier, LPDRAWITEMSTRUCT lpDrawItem)
{
	int index, button;
	
	if ((identifier & 0xF0) == 0x00)
	{
		index = (identifier & 0x0F) / 4;
		button = (identifier & 0x0F) % 4;
	}
	else if ((identifier & 0xF0) == 0x10)
	{
		index = 2;
		button = (identifier & 0x0F);
	}
	else
	{
		// Unknown button set.
		return;
	}
	
	HDC hDC = lpDrawItem->hDC;
	RECT itemRect = lpDrawItem->rcItem;
	
	if (index < 2)
		FillRect(hDC, &itemRect, brushOSD[button][0]);
	else //if (index == 2)
		FillRect(hDC, &itemRect, brushIntroEffect[button][0]);
	
	// Check if the radio button is checked.
	if (state_optColor[index] == button)
	{
		// Checked. Draw a circle in the middle of the box.
		
		if (index < 2)
		{
			SelectBrush(hDC, brushOSD[button][1]);
			SelectPen(hDC, penOSD[button][1]);
		}
		else //if (index == 2)
		{
			SelectBrush(hDC, brushIntroEffect[button][1]);
			SelectPen(hDC, penIntroEffect[button][1]);
		}
		
		Ellipse(hDC, itemRect.left + 4, itemRect.top + 4,
			     itemRect.right - 4, itemRect.bottom - 4);
	}
}


static void genopt_window_callback_selectRadioButton(int identifier)
{
	int index, button;
	
	if ((identifier & 0xF0) == 0x00)
	{
		index = (identifier & 0x0F) / 4;
		button = (identifier & 0x0F) % 4;
	}
	else if ((identifier & 0xF0) == 0x10)
	{
		index = 2;
		button = (identifier & 0x0F);
	}
	else
	{
		// Unknown button set.
		return;
	}
	
	// Make sure the index and button IDs are valid.
	if (index < 2 && button >= 4)
		return;
	else if (index == 2 && button >= 8)
		return;
	else if (index > 2)
		return;
	
	if (state_optColor[index] == button)
	{
		// State hasn't changed. Don't do anything.
		return;
	}
	
	// State has changed.
	unsigned short oldButton = state_optColor[index];
	state_optColor[index] = button;
	if (index < 2)
	{
		InvalidateRect(optOSD_Color[index][oldButton], NULL, FALSE);
		InvalidateRect(optOSD_Color[index][button], NULL, FALSE);
	}
	else //if (index == 2)
	{
		InvalidateRect(optIntroEffectColor[oldButton], NULL, FALSE);
		InvalidateRect(optIntroEffectColor[button], NULL, FALSE);
	}
}
