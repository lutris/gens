/***************************************************************************
 * Gens: (Win32) General Options Window.                                   *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2010 by David Korth                                  *
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
#include "btncolor.hpp"
#include "gens/gens_window.h"

// C includes.
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

// Win32 includes.
#include "libgsft/w32u/w32u_windows.h"
#include "libgsft/w32u/w32u_windowsx.h"
#include "libgsft/w32u/w32u_commctrl.h"
#include "libgsft/w32u/w32u_commdlg.h"
#include "ui/win32/resource.h"

// libgsft includes.
#include "libgsft/gsft_win32.h"
#include "libgsft/gsft_win32_gdi.h"

// Main settings.
#include "emulator/g_main.hpp"
#include "emulator/options.hpp"
#include "gens_ui.hpp"

// Video and Audio functions.
#include "video/vdraw.h"
#include "audio/audio.h"

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

// Window size. (NOTE: THESE ARE IN DIALOG UNITS, and must be converted to pixels using DLU_X() / DLU_Y().)
#define GENOPT_WINDOW_WIDTH  225
#define GENOPT_WINDOW_HEIGHT 180

#define GENOPT_OSD_FRAME_WIDTH  95
#define GENOPT_OSD_FRAME_HEIGHT 65

// Window procedure.
static LRESULT CALLBACK genopt_window_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// Widgets.
static HWND	btnOK, btnCancel, btnApply;

// Widgets: On-Screen Display.
static HWND	chkOSD_Enable[2];
static HWND	chkOSD_DoubleSized[2];
static HWND	chkOSD_Transparency[2];
static HWND	btnColor[2];
static BtnColor *btnColor_cpp[2] = {NULL, NULL};

// NOTE: Win32 colors have the Red and Blue components
// swapped compared to the internal colors.
static uint32_t	osd_colors[2];

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
static HWND	chkMisc_ntscV30rolling;

// GDI objects.
static HBRUSH	brushIntroEffect[8][2];
static HPEN	penIntroEffect[8][2];

// Widget creation functions.
static void WINAPI genopt_window_create_child_windows(HWND hWnd);
static void WINAPI genopt_window_create_osd_frame(HWND container, const char* title, const int index,
							int x, int y, const int w, const int h);

// Configuration load/save functions.
static void WINAPI genopt_window_init(void);
static void WINAPI genopt_window_save(void);

// Callbacks.
static void WINAPI genopt_window_callback_drawMsgRadioButton(int identifier, LPDRAWITEMSTRUCT lpDrawItem);
static void WINAPI genopt_window_callback_selectRadioButton(int identifier);
static void WINAPI genopt_window_callback_btnColor_clicked(int identifier);


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
		genopt_wndclass.hIcon = LoadIconA(ghInstance, MAKEINTRESOURCE(IDI_GENS_APP));
		genopt_wndclass.hCursor = LoadCursorA(NULL, IDC_ARROW);
		genopt_wndclass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
		genopt_wndclass.lpszMenuName = NULL;
		genopt_wndclass.lpszClassName = "genopt_window";
		
		pRegisterClassU(&genopt_wndclass);
	}
	
	// Create the window.
	genopt_window = pCreateWindowU("genopt_window", "General Options",
					WS_DLGFRAME | WS_POPUP | WS_SYSMENU | WS_CAPTION,
					CW_USEDEFAULT, CW_USEDEFAULT,
					DLU_X(GENOPT_WINDOW_WIDTH), DLU_Y(GENOPT_WINDOW_HEIGHT),
					gens_window, NULL, ghInstance, NULL);
	
	// Set the actual window size.
	gsft_win32_set_actual_window_size(genopt_window, DLU_X(GENOPT_WINDOW_WIDTH), DLU_Y(GENOPT_WINDOW_HEIGHT));
	
	// Center the window on the Gens window.
	gsft_win32_center_on_window(genopt_window, gens_window);
	
	UpdateWindow(genopt_window);
	ShowWindow(genopt_window, SW_SHOW);
}


/**
 * genopt_window_create_child_windows(): Create child windows.
 * @param hWnd HWND of the parent window.
 */
static void WINAPI genopt_window_create_child_windows(HWND hWnd)
{
	HWND grpBox;
	
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
	HWND fraOSD = pCreateWindowU(WC_BUTTON, "On-Screen Display",
					WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
					DLU_X(5), DLU_Y(5),
					DLU_X(GENOPT_OSD_FRAME_WIDTH+10), DLU_Y((GENOPT_OSD_FRAME_HEIGHT*2)+5+5+5),
					hWnd, NULL, ghInstance, NULL);
	SetWindowFontU(fraOSD, w32_fntMessage, true);
	
	// FPS counter frame
	genopt_window_create_osd_frame(hWnd, "FPS counter", 0, DLU_X(10), DLU_Y(15),
					DLU_X(GENOPT_OSD_FRAME_WIDTH), DLU_Y(GENOPT_OSD_FRAME_HEIGHT));
	
	// Message counter frame
	genopt_window_create_osd_frame(hWnd, "Message", 1, DLU_X(10), DLU_Y(5+GENOPT_OSD_FRAME_HEIGHT+10),
					DLU_X(GENOPT_OSD_FRAME_WIDTH), DLU_Y(GENOPT_OSD_FRAME_HEIGHT));
	
	// Miscellaneous frame
	int frameLeft = DLU_X(5+GENOPT_OSD_FRAME_WIDTH+10+5);
	int frameTop = DLU_Y(5);
	
	grpBox = pCreateWindowU(WC_BUTTON, "Miscellaneous",
				WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
				frameLeft, frameTop,
				DLU_X(GENOPT_OSD_FRAME_WIDTH+10), DLU_Y(GENOPT_WINDOW_HEIGHT-5-14-10),
				hWnd, NULL, ghInstance, NULL);
	SetWindowFontU(grpBox, w32_fntMessage, true);
	
	// Auto Fix Checksum
	frameLeft += DLU_X(5);
	frameTop += DLU_Y(10);
	chkMisc_AutoFixChecksum = pCreateWindowU(WC_BUTTON, "Auto Fix Checksum",
							WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
							frameLeft, frameTop,
							DLU_X(GENOPT_OSD_FRAME_WIDTH), DLU_Y(12),
							hWnd, NULL, ghInstance, NULL);
	SetWindowFontU(chkMisc_AutoFixChecksum, w32_fntMessage, true);
	
	// Auto Pause
	frameTop += DLU_Y(12);
	chkMisc_AutoPause = pCreateWindowU(WC_BUTTON, "Auto Pause",
						WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
						frameLeft, frameTop,
						DLU_X(GENOPT_OSD_FRAME_WIDTH), DLU_Y(12),
						hWnd, NULL, ghInstance, NULL);
	SetWindowFontU(chkMisc_AutoPause, w32_fntMessage, true);
	
	// Fast Blur
	frameTop += DLU_Y(12);
	chkMisc_FastBlur = pCreateWindowU(WC_BUTTON, "Fast Blur",
						WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
						frameLeft, frameTop,
						DLU_X(GENOPT_OSD_FRAME_WIDTH), DLU_Y(12),
						hWnd, NULL, ghInstance, NULL);
	SetWindowFontU(chkMisc_FastBlur, w32_fntMessage, true);
	
	// Show SegaCD LEDs
	frameTop += DLU_Y(12);
	chkMisc_SegaCDLEDs = pCreateWindowU(WC_BUTTON, "Show SegaCD LEDs",
						WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
						frameLeft, frameTop,
						DLU_X(GENOPT_OSD_FRAME_WIDTH), DLU_Y(12),
						hWnd, NULL, ghInstance, NULL);
	SetWindowFontU(chkMisc_SegaCDLEDs, w32_fntMessage, true);
	
	// Border Color Emulation
	frameTop += DLU_Y(12);
	chkMisc_BorderColorEmulation = pCreateWindowU(WC_BUTTON, "Border Color Emulation",
							WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
							frameLeft, frameTop,
							DLU_X(GENOPT_OSD_FRAME_WIDTH), DLU_Y(12),
							hWnd, NULL, ghInstance, NULL);
	SetWindowFontU(chkMisc_BorderColorEmulation, w32_fntMessage, true);
	
	// Pause Tint
	frameTop += DLU_Y(12);
	chkMisc_PauseTint = pCreateWindowU(WC_BUTTON, "Pause Tint",
						WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
						frameLeft, frameTop,
						DLU_X(GENOPT_OSD_FRAME_WIDTH), DLU_Y(12),
						hWnd, NULL, ghInstance, NULL);
	SetWindowFontU(chkMisc_PauseTint, w32_fntMessage, true);
	
	// NTSC V30 Rolling
	frameTop += 20;
	chkMisc_ntscV30rolling = pCreateWindowU(WC_BUTTON, "NTSC V30 Rolling",
						WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
						frameLeft+8, frameTop,
						(GENOPT_OSD_FRAME_WIDTH-16), 20,
						hWnd, NULL, ghInstance, NULL);
	SetWindowFontU(chkMisc_ntscV30rolling, w32_fntMessage, true);
	
	// Intro Effect label.
	frameTop += DLU_Y(10+5);
	HWND lblIntroEffect = pCreateWindowU(WC_STATIC, "Intro Effect:",
						WS_CHILD | WS_VISIBLE | SS_CENTER,
						frameLeft, frameTop,
						DLU_X(GENOPT_OSD_FRAME_WIDTH), DLU_Y(12),
						hWnd, NULL, ghInstance, NULL);
	SetWindowFontU(lblIntroEffect, w32_fntMessage, true);
	
	// Dropdown for intro effect.
	frameTop += DLU_Y(10);
	cboIntroEffect = pCreateWindowU(WC_COMBOBOX, NULL,
					WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
					frameLeft, frameTop,
					DLU_X(GENOPT_OSD_FRAME_WIDTH), DLU_Y(14*3),
					hWnd, NULL, ghInstance, NULL);
	SetWindowFontU(cboIntroEffect, w32_fntMessage, true);
	ComboBox_AddStringU(cboIntroEffect, "None");
	ComboBox_AddStringU(cboIntroEffect, "Gens Logo Effect");
	ComboBox_AddStringU(cboIntroEffect, "\"Crazy\" Effect");
	//ComboBox_AddStringU(cboIntroEffect, "Genesis TMSS"); // TODO: Broken.
	
	// Intro effect color label
	frameTop += DLU_Y(15+2);
	HWND lblIntroEffectColor = pCreateWindowU(WC_STATIC, "Intro Effect Color:",
							WS_CHILD | WS_VISIBLE | SS_CENTER,
							frameLeft, frameTop,
							DLU_X(GENOPT_OSD_FRAME_WIDTH), DLU_Y(12),
							hWnd, NULL, ghInstance, NULL);
	SetWindowFontU(lblIntroEffectColor, w32_fntMessage, true);
	
	// Intro effect color buttons.
	frameLeft += ((DLU_X(GENOPT_OSD_FRAME_WIDTH) - (4*(16+8)-8)) / 2);
	frameTop += DLU_Y(12);
	for (int i = 0; i < 8; i++)
	{
		optIntroEffectColor[i] = pCreateWindowU(
				WC_STATIC, NULL, WS_CHILD | WS_VISIBLE | SS_CENTER | SS_OWNERDRAW | SS_NOTIFY,
				frameLeft + ((i%4)*(16+7)), frameTop + ((i / 4) * (16+4)), 16, 16,
				hWnd, (HMENU)(0xA010 + i), ghInstance, NULL);
	}
	
	// Create the dialog buttons.
	const int btnTop = DLU_Y(GENOPT_WINDOW_HEIGHT-5-14);
	int btnLeft = DLU_X(GENOPT_WINDOW_WIDTH-5-50-5-50-5-50);
	const int btnInc = DLU_X(5+50);
	
	// OK button.
	btnOK = pCreateWindowU(WC_BUTTON, "&OK",
				WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
				btnLeft, btnTop,
				DLU_X(50), DLU_Y(14),
				hWnd, (HMENU)IDOK, ghInstance, NULL);
	SetWindowFontU(btnOK, w32_fntMessage, true);
	
	// Cancel button.
	btnLeft += btnInc;
	btnCancel = pCreateWindowU(WC_BUTTON, "&Cancel",
					WS_CHILD | WS_VISIBLE | WS_TABSTOP,
					btnLeft, btnTop,
					DLU_X(50), DLU_Y(14),
					hWnd, (HMENU)IDCANCEL, ghInstance, NULL);
	SetWindowFontU(btnCancel, w32_fntMessage, true);
	
	// Apply button.
	btnLeft += btnInc;
	btnApply = pCreateWindowU(WC_BUTTON, "&Apply",
					WS_CHILD | WS_VISIBLE | WS_TABSTOP,
					btnLeft, btnTop,
					DLU_X(50), DLU_Y(14),
					hWnd, (HMENU)IDAPPLY, ghInstance, NULL);
	SetWindowFontU(btnApply, w32_fntMessage, true);
	
	// Initialize the internal data variables.
	genopt_window_init();
	
	// Set focus to "FPS counter" - "Enable".
	SetFocus(chkOSD_Enable[0]);
}


static void WINAPI genopt_window_create_osd_frame(HWND container, const char* title, const int index,
							int x, int y, const int w, const int h)
{
	// Message frame
	HWND grpBox, lblColor;
	
	grpBox = pCreateWindowU(WC_BUTTON, title,
				WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_GROUPBOX,
				x, y, w, h,
				container, NULL, ghInstance, NULL);
	SetWindowFontU(grpBox, w32_fntMessage, true);
	
	x += DLU_X(5);
	y += DLU_Y(10);
	const int rowInc = DLU_Y(12);
	
	// Enable
	chkOSD_Enable[index] = pCreateWindowU(WC_BUTTON, "Enable",
						WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
						x, y,
						DLU_X(GENOPT_OSD_FRAME_WIDTH-10), DLU_Y(10),
						container, NULL, ghInstance, NULL);
	SetWindowFontU(chkOSD_Enable[index], w32_fntMessage, true);
	
	// Double Sized
	y += rowInc;
	chkOSD_DoubleSized[index] = pCreateWindowU(WC_BUTTON, "Double Sized",
							WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
							x, y,
							DLU_X(GENOPT_OSD_FRAME_WIDTH-10), DLU_Y(10),
							container, NULL, ghInstance, NULL);
	SetWindowFontU(chkOSD_DoubleSized[index], w32_fntMessage, true);
	
	// Transparency
	y += rowInc;
	chkOSD_Transparency[index] = pCreateWindowU(WC_BUTTON, "Transparency",
							WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
							x, y,
							DLU_X(GENOPT_OSD_FRAME_WIDTH-10), DLU_Y(10),
							container, NULL, ghInstance, NULL);
	SetWindowFontU(chkOSD_Transparency[index], w32_fntMessage, true);
	
	// Color label.
	y += rowInc;
	lblColor = pCreateWindowU(WC_STATIC, "Color:",
					WS_CHILD | WS_VISIBLE | SS_LEFT,
					x, y+DLU_Y(2),
					DLU_X(22), DLU_Y(10),
					container, NULL, ghInstance, NULL);
	SetWindowFontU(lblColor, w32_fntMessage, true);
	
	// Color button.
	btnColor[index] = pCreateWindowU(WC_BUTTON, "Change...",
						WS_CHILD | WS_VISIBLE | WS_TABSTOP,
						x+DLU_X(5+22), y,
						DLU_X(GENOPT_OSD_FRAME_WIDTH-10-22-5), DLU_Y(14),
						container, (HMENU)(0xB000 + index), ghInstance, NULL);
	SetWindowFontU(btnColor[index], w32_fntMessage, true);
	
	// Color button C++ wrapper.
	btnColor_cpp[index] = new BtnColor(btnColor[index]);
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
	
	// Delete intro effect color brushes and pens.
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			DeleteBrush(brushIntroEffect[i][j]);
			DeletePen(penIntroEffect[i][j]);
		}
	}
	
	// Delete the color button C++ wrappers.
	for (int i = 0; i < 2; i++)
	{
		delete btnColor_cpp[i];
		btnColor_cpp[i] = NULL;
	}
}


/**
 * genopt_window_init(): Initialize the internal variables.
 */
static void WINAPI genopt_window_init(void)
{
	// Get the current options.
	unsigned char curFPSStyle, curMsgStyle;
	uint32_t color_tmp;
	
	// Miscellaneous
	Button_SetCheckU(chkMisc_AutoFixChecksum, (Auto_Fix_CS ? BST_CHECKED : BST_UNCHECKED));
	Button_SetCheckU(chkMisc_AutoPause, (Auto_Pause ? BST_CHECKED : BST_UNCHECKED));
	Button_SetCheckU(chkMisc_FastBlur, (vdraw_get_fast_blur() ? BST_CHECKED : BST_UNCHECKED));
	Button_SetCheckU(chkMisc_SegaCDLEDs, (Show_LED ? BST_CHECKED : BST_UNCHECKED));
	Button_SetCheckU(chkMisc_BorderColorEmulation, (Video.borderColorEmulation ? BST_CHECKED : BST_UNCHECKED));
	Button_SetCheckU(chkMisc_PauseTint, (Video.pauseTint ? BST_CHECKED : BST_UNCHECKED));
	Button_SetCheckU(chkMisc_ntscV30rolling, (Video.ntscV30rolling ? BST_CHECKED : BST_UNCHECKED));
	
	// FPS counter
	Button_SetCheckU(chkOSD_Enable[0], (vdraw_get_fps_enabled() ? BST_CHECKED : BST_UNCHECKED));
	
	curFPSStyle = vdraw_get_fps_style();
	Button_SetCheckU(chkOSD_DoubleSized[0], ((curFPSStyle & 0x10) ? BST_CHECKED : BST_UNCHECKED));
	Button_SetCheckU(chkOSD_Transparency[0], ((curFPSStyle & 0x08) ? BST_CHECKED : BST_UNCHECKED));
	
	color_tmp = vdraw_get_fps_color();
	osd_colors[0] = ((color_tmp >> 16) & 0xFF) |
			((color_tmp & 0xFF00)) |
			((color_tmp & 0xFF) << 16);
	btnColor_cpp[0]->setBgColor(osd_colors[0]);
	
	// Message
	Button_SetCheckU(chkOSD_Enable[1], (vdraw_get_msg_enabled() ? BST_CHECKED : BST_UNCHECKED));
	
	curMsgStyle = vdraw_get_msg_style();
	Button_SetCheckU(chkOSD_DoubleSized[1], ((curMsgStyle & 0x10) ? BST_CHECKED : BST_UNCHECKED));
	Button_SetCheckU(chkOSD_Transparency[1], ((curMsgStyle & 0x08) ? BST_CHECKED : BST_UNCHECKED));
	
	color_tmp = vdraw_get_msg_color();
	osd_colors[1] = ((color_tmp >> 16) & 0xFF) |
			((color_tmp & 0xFF00)) |
			((color_tmp & 0xFF) << 16);
	btnColor_cpp[1]->setBgColor(osd_colors[1]);
	
	// Intro effect.
	ComboBox_SetCurSelU(cboIntroEffect, Intro_Style);
	if (ComboBox_GetCurSelU(cboIntroEffect) == -1)
		ComboBox_SetCurSelU(cboIntroEffect, 0);
	state_optColor[2] = vdraw_get_intro_effect_color();
}


/**
 * genopt_window_save(): Save the settings.
 */
static void WINAPI genopt_window_save(void)
{
	// Save the current options.
	unsigned char curFPSStyle, curMsgStyle;
	uint32_t color_tmp;
	
	// System
	Auto_Fix_CS = (Button_GetCheckU(chkMisc_AutoFixChecksum) == BST_CHECKED);
	Auto_Pause = (Button_GetCheckU(chkMisc_AutoPause) == BST_CHECKED);
	vdraw_set_fast_blur(Button_GetCheckU(chkMisc_FastBlur) == BST_CHECKED);
	Show_LED = (Button_GetCheckU(chkMisc_SegaCDLEDs) == BST_CHECKED);
	Video.borderColorEmulation = (Button_GetCheckU(chkMisc_BorderColorEmulation) == BST_CHECKED);
	Video.pauseTint = (Button_GetCheckU(chkMisc_PauseTint) == BST_CHECKED);
	Video.ntscV30rolling = (Button_GetCheckU(chkMisc_ntscV30rolling) == BST_CHECKED);
	
	// If Auto Pause is enabled, deactivate emulation.
	// If Auto Pause is disabled, activate emulation.
	Settings.Active = !Auto_Pause;
	
	// FPS counter
	vdraw_set_fps_enabled(Button_GetCheckU(chkOSD_Enable[0]) == BST_CHECKED);
	
	curFPSStyle = vdraw_get_fps_style() & ~0x18;
	curFPSStyle |= ((Button_GetCheckU(chkOSD_DoubleSized[0]) == BST_CHECKED) ? 0x10 : 0x00);
	curFPSStyle |= ((Button_GetCheckU(chkOSD_Transparency[0]) == BST_CHECKED) ? 0x08 : 0x00);
	curFPSStyle &= ~0x06;
	vdraw_set_fps_style(curFPSStyle);
	
	color_tmp = ((osd_colors[0] >> 16) & 0xFF) |
		    ((osd_colors[0] & 0xFF00)) |
		    ((osd_colors[0] & 0xFF) << 16);
	vdraw_set_fps_color(color_tmp);
	
	// Message
	vdraw_set_msg_enabled(Button_GetCheckU(chkOSD_Enable[1]) == BST_CHECKED);
	curMsgStyle = vdraw_get_msg_style() & ~0x18;
	curMsgStyle |= ((Button_GetCheckU(chkOSD_DoubleSized[1]) == BST_CHECKED) ? 0x10 : 0x00);
	curMsgStyle |= ((Button_GetCheckU(chkOSD_Transparency[1]) == BST_CHECKED) ? 0x08 : 0x00);
	curMsgStyle &= ~0x06;
	vdraw_set_msg_style(curMsgStyle);
	
	color_tmp = ((osd_colors[1] >> 16) & 0xFF) |
		    ((osd_colors[1] & 0xFF00)) |
		    ((osd_colors[1] & 0xFF) << 16);
	vdraw_set_msg_color(color_tmp);

	// Intro effect.
	Intro_Style = ComboBox_GetCurSelU(cboIntroEffect);
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
			switch (LOWORD(wParam) & 0xFF00)
			{
				case 0xA000:
					genopt_window_callback_drawMsgRadioButton(LOWORD(wParam), (LPDRAWITEMSTRUCT)lParam);
					break;
				case 0xB000:
				{
					int id = (LOWORD(wParam) & 0xFF);
					if (id >= 0 && id < 2)
						btnColor_cpp[id]->handleDrawItem((LPDRAWITEMSTRUCT)lParam);
					break;
				}
				default:
					break;
			}
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
					else if ((LOWORD(wParam) & 0xFF00) == 0xB000)
					{
						// Change Color button.
						genopt_window_callback_btnColor_clicked(LOWORD(wParam));
					}
					break;
			}
			break;
		
		case WM_DESTROY:
			if (hWnd != genopt_window)
				break;
			
			genopt_window = NULL;
			
			// Delete intro effect color brushes and pens.
			for (int i = 0; i < 8; i++)
			{
				for (int j = 0; j < 2; j++)
				{
					DeleteBrush(brushIntroEffect[i][j]);
					DeletePen(penIntroEffect[i][j]);
				}
			}
			
			// Delete the color button C++ wrappers.
			for (int i = 0; i < 2; i++)
			{
				delete btnColor_cpp[i];
				btnColor_cpp[i] = NULL;
			}
			
			break;
		
		default:
			break;
	}
	
	return pDefWindowProcU(hWnd, message, wParam, lParam);
}


static void WINAPI genopt_window_callback_drawMsgRadioButton(int identifier, LPDRAWITEMSTRUCT lpDrawItem)
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
	
	if (index == 2)
		FillRect(hDC, &itemRect, brushIntroEffect[button][0]);
	
	// Check if the radio button is checked.
	if (state_optColor[index] == button)
	{
		// Checked. Draw a circle in the middle of the box.
		if (index == 2)
		{
			SelectBrush(hDC, brushIntroEffect[button][1]);
			SelectPen(hDC, penIntroEffect[button][1]);
		}
		
		Ellipse(hDC, itemRect.left + 4, itemRect.top + 4,
			     itemRect.right - 4, itemRect.bottom - 4);
	}
}


static void WINAPI genopt_window_callback_selectRadioButton(int identifier)
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
	if (index == 2)
	{
		InvalidateRect(optIntroEffectColor[oldButton], NULL, false);
		InvalidateRect(optIntroEffectColor[button], NULL, false);
	}
}


/**
 * genopt_window_callback_btnColor_clicked(): "Change..." button for a color was clicked.
 * @param identifier Button identifier.
 */
static void WINAPI genopt_window_callback_btnColor_clicked(int identifier)
{
	identifier &= 0x0F;
	if (identifier >= 2)
		return;
	
	// Custom colors.
	// NOTE: These are only preserved for the current session.
	static COLORREF custom_colors[16] =
	{
		RGB(255, 255, 255), RGB(255, 255, 255),
		RGB(255, 255, 255), RGB(255, 255, 255),
		RGB(255, 255, 255), RGB(255, 255, 255),
		RGB(255, 255, 255), RGB(255, 255, 255),
		RGB(255, 255, 255), RGB(255, 255, 255),
		RGB(255, 255, 255), RGB(255, 255, 255),
		RGB(255, 255, 255), RGB(255, 255, 255),
		RGB(255, 255, 255), RGB(255, 255, 255),
	};
	
	// Create a color picker dialog.
	CHOOSECOLOR color_sel;
	memset(&color_sel, 0x00, sizeof(color_sel));
	color_sel.lStructSize = sizeof(color_sel);
	color_sel.hwndOwner = genopt_window;
	color_sel.rgbResult = osd_colors[identifier];
	color_sel.lpCustColors = custom_colors;
	color_sel.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;
	
	// Request a color.
	// TODO: w32u version of ChooseColor()
	audio_clear_sound_buffer();
	BOOL bRet = ChooseColor(&color_sel);
	if (bRet == 0)
	{
		// No color selected.
		return;
	}
	
	// Color selected.
	if (color_sel.rgbResult == osd_colors[identifier])
		return;
	
	// Set the new color.
	osd_colors[identifier] = color_sel.rgbResult;
	btnColor_cpp[identifier]->setBgColor(osd_colors[identifier]);
	
	// Enable the "Apply" button.
	Button_Enable(btnApply, true);
}
