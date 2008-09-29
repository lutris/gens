/***************************************************************************
 * Gens: (Win32) About Window.                                             *
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

#include "about_window.h"
#include "gens/gens_window.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "emulator/g_main.hpp"

WNDCLASS WndClass;
HWND about_window = NULL;

// Labels
HWND lblGensTitle = NULL;
HWND lblGensDesc = NULL;

// Gens logo
HBITMAP bmpGensLogo = NULL;

// Gens Win32 resources
#include "ui/win32/resource.h"

#include "ui/about_window_data.h"
//GtkWidget *image_gens_logo = NULL;
#define ID_TIMER_ICE 0x1234
UINT_PTR tmrIce = NULL;
void updateIce(void);
void iceTime(void);

const unsigned short iceOffsetX = 20;
const unsigned short iceOffsetY = 8;
int iceLastTicks = 0;
int ax = 0, bx = 0, cx = 0;

LRESULT CALLBACK About_Window_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static void About_Window_CreateChildWindows(HWND hWnd);


/**
 * create_about_window(): Create the About Window.
 * @return About Window.
 */
HWND create_about_window(void)
{
	if (about_window)
	{
		// About window is already created. Set focus.
		// TODO: Figure out how to do this.
		ShowWindow(about_window, 1);
		return about_window;
	}
	
	WndClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = About_Window_WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = ghInstance;
	WndClass.hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_SONIC_HEAD));
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = "Gens_About";
	
	RegisterClass(&WndClass);
	
	about_window = CreateWindowEx(NULL, "Gens_About", "About Gens",
				      (WS_POPUP | WS_SYSMENU | WS_CAPTION) & ~(WS_MINIMIZE),
				      CW_USEDEFAULT, CW_USEDEFAULT,
				      320 + Win32_dw, 320 + Win32_dh, NULL, NULL, ghInstance, NULL);
	
#if 0
	// Gens logo
	image_gens_logo = create_pixmap("gens_small.png");
	gtk_widget_set_name(image_gens_logo, "image_gens_logo");
	gtk_widget_show(image_gens_logo);
	gtk_box_pack_start(GTK_BOX(hbox_about_logo), image_gens_logo, TRUE, TRUE, 0);
	GLADE_HOOKUP_OBJECT(about_window, image_gens_logo, "image_gens_logo");
#endif

#if 0
	// Get the dialog VBox.
	vbox_about_dialog = GTK_DIALOG(about_window)->vbox;
	gtk_widget_set_name(vbox_about_dialog, "vbox_about_dialog");
	gtk_widget_show(vbox_about_dialog);
	GLADE_HOOKUP_OBJECT_NO_REF(about_window, vbox_about_dialog, "vbox_about_dialog");
	
	// Create the main VBox.
	vbox_about_main = gtk_vbox_new(FALSE, 5);
	gtk_widget_set_name(vbox_about_main, "vbox_about_main");
	gtk_widget_show(vbox_about_main);
	gtk_container_add(GTK_CONTAINER(vbox_about_dialog), vbox_about_main);
	GLADE_HOOKUP_OBJECT(about_window, vbox_about_main, "vbox_about_main");
	
	// Create the HBox for the logo and the program information.
	hbox_about_logo = gtk_hbox_new(FALSE, 0);
	gtk_widget_set_name(hbox_about_logo, "hbox_about_logo");
	gtk_widget_show(hbox_about_logo);
	gtk_box_pack_start(GTK_BOX(vbox_about_main), hbox_about_logo, TRUE, TRUE, 0);
	GLADE_HOOKUP_OBJECT(about_window, hbox_about_logo, "hbox_about_logo");
	
	// Version information
	label_gens_version = gtk_label_new(
		"<b><i>Gens for Linux\n"
		"Version " GENS_VERSION "</i></b>\n\n"
		"Sega Genesis / Mega Drive,\n"
		"Sega CD / Mega CD,\n"
		"Sega 32X emulator"
		);
	gtk_widget_set_name(label_gens_version, "label_gens_version");
	gtk_label_set_use_markup(GTK_LABEL(label_gens_version), TRUE);
	gtk_label_set_justify(GTK_LABEL(label_gens_version), GTK_JUSTIFY_CENTER);
	gtk_widget_show(label_gens_version);
	gtk_box_pack_start(GTK_BOX(hbox_about_logo), label_gens_version, FALSE, FALSE, 0);
	GLADE_HOOKUP_OBJECT(about_window, label_gens_version, "label_gens_version");
	
	// Copyright frame
	frame_copyright = gtk_frame_new(NULL);
	gtk_widget_set_name(frame_copyright, "frame_copyright");
	gtk_container_set_border_width(GTK_CONTAINER(frame_copyright), 5);
	gtk_frame_set_shadow_type(GTK_FRAME(frame_copyright), GTK_SHADOW_NONE);
	gtk_widget_show(frame_copyright);
	gtk_box_pack_start(GTK_BOX(vbox_about_main), frame_copyright, FALSE, FALSE, 0);
	GLADE_HOOKUP_OBJECT(about_window, frame_copyright, "frame_copyright");
	
	// Copyright label
	label_copyright = gtk_label_new (
		"(c) 1999-2002 by Stéphane Dallongeville\n"
		"(c) 2003-2004 by Stéphane Akhoun\n\n"
		"Gens/GS (c) 2008 by David Korth\n\n"
		"Visit the Gens homepage:\n"
		"http://gens.consolemul.com\n\n"
		"For news on Gens/GS, visit Sonic Retro:\n"
		"http://www.sonicretro.org"
		);
	gtk_widget_set_name(label_copyright, "label_copyright");
	gtk_widget_show(label_copyright);
	gtk_container_add(GTK_CONTAINER(frame_copyright), label_copyright);
	GLADE_HOOKUP_OBJECT(about_window, label_copyright, "label_copyright");
	
	// Get the dialog action area.
	about_dialog_action_area = GTK_DIALOG(about_window)->action_area;
	gtk_widget_set_name(about_dialog_action_area, "about_dialog_action_area");
	gtk_widget_show(about_dialog_action_area);
	GLADE_HOOKUP_OBJECT_NO_REF(about_window, about_dialog_action_area, "about_dialog_action_area");
	
	// Add the OK button to the dialog action area.
	button_about_OK = gtk_button_new_from_stock("gtk-ok");
	gtk_widget_set_name(button_about_OK, "button_about_OK");
	GTK_WIDGET_SET_FLAGS(button_about_OK, GTK_CAN_DEFAULT);
	gtk_widget_show(button_about_OK);
	gtk_dialog_add_action_widget(GTK_DIALOG(about_window), button_about_OK, GTK_RESPONSE_OK);
	AddButtonCallback_Clicked(button_about_OK, on_button_about_OK_clicked);
	gtk_widget_add_accelerator(button_about_OK, "activate", accel_group,
				   GDK_Return, (GdkModifierType)(0), (GtkAccelFlags)(0));
	gtk_widget_add_accelerator(button_about_OK, "activate", accel_group,
				   GDK_KP_Enter, (GdkModifierType)(0), (GtkAccelFlags)(0));
	GLADE_HOOKUP_OBJECT(about_window, button_about_OK, "button_about_OK");
	
	// Add the accel group.
	gtk_window_add_accel_group(GTK_WINDOW(about_window), accel_group);
	
	gtk_widget_show_all(about_window);
#endif
	
	ShowWindow(about_window, 1);
	UpdateWindow(about_window);
	return about_window;
}


/**
 * About_Window_WndProc(): The About window procedure.
 * @param hWnd hWnd of the object sending a message.
 * @param message Message being sent by the object.
 * @param wParam
 * @param lParam
 * @return
 */
LRESULT CALLBACK About_Window_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_CREATE:
			About_Window_CreateChildWindows(hWnd);
			break;
		
		case WM_CLOSE:
			DestroyWindow(about_window);
			return 0;
		
		case WM_PAINT:
			if (ice == 3)
				updateIce();
			break;
		
		case WM_TIMER:
			if (wParam == ID_TIMER_ICE && ice == 3)
				iceTime();
			break;
		
		case WM_CTLCOLORSTATIC:
			if (hWnd != about_window)
				break;
			
			// Set the title and version labels to transparent.
			if ((HWND)lParam == lblGensTitle ||
			    (HWND)lParam == lblGensDesc)
			{
				SetBkMode((HDC)wParam, TRANSPARENT);
				return (LRESULT)GetStockObject(NULL_BRUSH);
			}
			return TRUE;
			break;
		
		case WM_DESTROY:
			if (hWnd != about_window)
				break;
			
			if (tmrIce)
			{
				KillTimer(about_window, tmrIce);
				tmrIce = 0;
			}
			if (bmpGensLogo)
			{
				DeleteObject(bmpGensLogo);
				bmpGensLogo = NULL;
			}
			lblGensTitle = NULL;
			lblGensDesc = NULL;
			about_window = NULL;
			break;
	}
	
	return DefWindowProc(hWnd, message, wParam, lParam);
}


static void About_Window_CreateChildWindows(HWND hWnd)
{
	Win32_centerOnGensWindow(hWnd);
	
	if (ice != 3)
	{
		// Gens logo
		HWND imgGensLogo;
		imgGensLogo = CreateWindow("Static", NULL, WS_CHILD | WS_VISIBLE | SS_BITMAP,
					   0, 0, 128, 96, hWnd, NULL, ghInstance, NULL);
		bmpGensLogo = LoadImage(ghInstance, MAKEINTRESOURCE(IDB_GENS_LOGO_SMALL),
					        IMAGE_BITMAP, 0, 0,
						LR_DEFAULTSIZE | LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT);
		SendMessage(imgGensLogo, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bmpGensLogo);
	}
	else
	{
		// "ice" timer
		ax = 0; bx = 0; cx = 1;
		tmrIce = SetTimer(hWnd, ID_TIMER_ICE, 10, NULL);
		updateIce();
	}
	
	// Title and version information.
	lblGensTitle = CreateWindow("Static", aboutTitle, WS_CHILD | WS_VISIBLE | SS_CENTER,
				    128, 8, 184, 24, hWnd, NULL, ghInstance, NULL);
	SendMessage(lblGensTitle, WM_SETFONT, (WPARAM)fntTitle, 1);
	
	lblGensDesc = CreateWindow("Static", aboutDesc, WS_CHILD | WS_VISIBLE | SS_CENTER,
				   128, 44, 184, 100, hWnd, NULL, ghInstance, NULL);
	SendMessage(lblGensDesc, WM_SETFONT, (WPARAM)fntMain, 1);
}


#if 0
/**
 * Window is closed.
 */
gboolean on_about_window_close(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(widget);
	GENS_UNUSED_PARAMETER(event);
	GENS_UNUSED_PARAMETER(user_data);
	
	cx = 0;
	gtk_widget_destroy(about_window);
	about_window = NULL;
	return FALSE;
}


/**
 * OK
 */
void on_button_about_OK_clicked(GtkButton *button, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(button);
	GENS_UNUSED_PARAMETER(user_data);
	
	cx = 0;
	gtk_widget_destroy(about_window);
	about_window = NULL;
}
#endif


void updateIce(void)
{
	HDC hDC;
	PAINTSTRUCT ps;
	
	hDC = BeginPaint(about_window, &ps);
	
	int x, y;
	const unsigned char *src = &about_data[ax*01440];
	const unsigned char *src2 = &about_dx[bx*040];
	unsigned char px1, px2;
	
	int bgc = GetSysColor(COLOR_3DFACE);
	int pxc;
	
	for (y = 0; y < 0120; y += 2)
	{
		for (x = 0; x < 0120; x += 4)
		{
			px1 = (*src & 0360) >> 3;
			px2 = (*src & 0017) << 1;
			
			if (!px1)
			{
				pxc = bgc;
			}
			else
			{
				pxc = RGB((src2[px1 + 1] & 0017) << 4,
					  (src2[px1 + 1] & 0360),
					  (src2[px1 + 0] & 0017) << 4);
			}
			
			SetPixel(hDC, x + 0 + iceOffsetX, y + 0 + iceOffsetY, pxc);
			SetPixel(hDC, x + 1 + iceOffsetX, y + 0 + iceOffsetY, pxc);
			SetPixel(hDC, x + 0 + iceOffsetX, y + 1 + iceOffsetY, pxc);
			SetPixel(hDC, x + 1 + iceOffsetX, y + 1 + iceOffsetY, pxc);
			
			if (!px2)
			{
				pxc = bgc;
			}
			else
			{
				pxc = RGB((src2[px2 + 1] & 0017) << 4,
					  (src2[px2 + 1] & 0360),
					  (src2[px2 + 0] & 0017) << 4);
			}
			SetPixel(hDC, x + 2 + iceOffsetX, y + 0 + iceOffsetY, pxc);
			SetPixel(hDC, x + 3 + iceOffsetX, y + 0 + iceOffsetY, pxc);
			SetPixel(hDC, x + 2 + iceOffsetX, y + 1 + iceOffsetY, pxc);
			SetPixel(hDC, x + 3 + iceOffsetX, y + 1 + iceOffsetY, pxc);
			
			src++;
		}
	}
	
	EndPaint(about_window, &ps);
}


void iceTime(void)
{
	if (iceLastTicks + 100 > GetTickCount())
		return;
	
	ax ^= 1;
	bx++;
	if (bx >= 10)
		bx = 0;
	
	// Force a repaint.
	RECT rIce;
	rIce.left = iceOffsetX;
	rIce.top = iceOffsetY;
	rIce.right = iceOffsetX + 80 - 1;
	rIce.bottom = iceOffsetY + 80 - 1;
	InvalidateRect(about_window, &rIce, FALSE);
	SendMessage(about_window, WM_PAINT, 0, 0);
	
	iceLastTicks = GetTickCount();
}
