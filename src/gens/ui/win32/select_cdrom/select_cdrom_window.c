/***************************************************************************
 * Gens: (Win32) Select CD-ROM Drive Window.                               *
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

#include "emulator/g_main.hpp"

#include "select_cdrom_window.h"
#include "select_cdrom_window_callbacks.h"
#include "gens/gens_window.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include <windowsx.h>

// Gens Win32 resources
#include "ui/win32/resource.h"

// Win32 common controls
#include <commctrl.h>

static WNDCLASS WndClass;
HWND select_cdrom_window = NULL;

// Controls
HWND SelCD_cdromDropdownBox = NULL;

/**
 * create_select_cdrom_window(): Create the Select CD-ROM Drive Window.
 * @return Select CD-ROM Drive Window.
 */
HWND create_select_cdrom_window(void)
{
	if (select_cdrom_window)
	{
		// Select CD-ROM Drive window is already created. Set focus.
		// TODO: Figure out how to do this.
		ShowWindow(select_cdrom_window, 1);
		return NULL;
	}
	
	// Create the window class.
	WndClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = Select_CDROM_Window_WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = ghInstance;
	WndClass.hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_SONIC_HEAD));
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = "Gens_Select_CDROM";
	
	RegisterClass(&WndClass);
	
	// Create the window.
	select_cdrom_window = CreateWindowEx(NULL, "Gens_Select_CDROM", "Select CD-ROM Drive",
					     WS_DLGFRAME | WS_POPUP | WS_SYSMENU | WS_CAPTION,
					     CW_USEDEFAULT, CW_USEDEFAULT,
					     288, 80,
					     Gens_hWnd, NULL, ghInstance, NULL);
	
	// Set the actual window size.
	Win32_setActualWindowSize(select_cdrom_window, 288, 68);
	
	// Center the window on the Gens window.
	Win32_centerOnGensWindow(select_cdrom_window);
	
	UpdateWindow(select_cdrom_window);
	return select_cdrom_window;
	
#if 0
	// Create the main VBox.
	vbox_SelCD = gtk_vbox_new(FALSE, 5);
	gtk_widget_set_name(vbox_SelCD, "vbox_SelCD");
	gtk_widget_show(vbox_SelCD);
	gtk_container_add(GTK_CONTAINER(select_cdrom_window), vbox_SelCD);
	GLADE_HOOKUP_OBJECT(select_cdrom_window, vbox_SelCD, "vbox_SelCD");
	
	// Add a frame for drive selection.
	frame_drive = gtk_frame_new(NULL);
	gtk_widget_set_name(frame_drive, "frame_drive");
	gtk_container_set_border_width(GTK_CONTAINER(frame_drive), 5);
	gtk_frame_set_shadow_type(GTK_FRAME(frame_drive), GTK_SHADOW_NONE);
	gtk_widget_show(frame_drive);
	gtk_box_pack_start(GTK_BOX(vbox_SelCD), frame_drive, FALSE, FALSE, 0);
	GLADE_HOOKUP_OBJECT(select_cdrom_window, frame_drive, "frame_drive");
	
	// Add a label to the drive selection frame.
	label_frame_drive = gtk_label_new(
		"Typical values are <i>/dev/cdrom</i>, "
		"<i>/dev/sr0</i>, or <i>/dev/hdc</i>"
		);
	gtk_widget_set_name(label_frame_drive, "label_frame_drive");
	gtk_label_set_use_markup(GTK_LABEL(label_frame_drive), TRUE);
	gtk_widget_show(label_frame_drive);
	gtk_frame_set_label_widget(GTK_FRAME(frame_drive), label_frame_drive);
	GLADE_HOOKUP_OBJECT(select_cdrom_window, label_frame_drive, "label_frame_drive");
	
	// HBox for the CD drive label and combo box.
	hbox_drive = gtk_hbox_new(FALSE, 0);
	gtk_widget_set_name(hbox_drive, "hbox_drive");
	gtk_widget_show(hbox_drive);
	gtk_container_add(GTK_CONTAINER(frame_drive), hbox_drive);
	GLADE_HOOKUP_OBJECT(select_cdrom_window, hbox_drive, "hbox_drive");
	
	// CD drive label
	label_drive = gtk_label_new_with_mnemonic("CD-_ROM drive:");
	gtk_widget_set_name(label_drive, "label_drive");
	gtk_widget_show(label_drive);
	gtk_box_pack_start(GTK_BOX(hbox_drive), label_drive, FALSE, FALSE, 0);
	GLADE_HOOKUP_OBJECT(select_cdrom_window, label_drive, "label_drive");
	
	// Add an editable combo box for the CD drive.
	combo_drive = gtk_combo_box_entry_new_text();
	gtk_widget_set_name(combo_drive, "combo_drive");
	gtk_widget_show(combo_drive);
	gtk_box_pack_start(GTK_BOX(hbox_drive), combo_drive, TRUE, TRUE, 0);
	gtk_entry_set_max_length(GTK_ENTRY(GTK_BIN(combo_drive)->child), 63);
	gtk_label_set_mnemonic_widget(GTK_LABEL(label_drive), combo_drive);
	GLADE_HOOKUP_OBJECT(select_cdrom_window, combo_drive, "combo_drive");
	
	// Add an HBox for speed selection.
	hbox_speed = gtk_hbox_new(FALSE, 0);
	gtk_widget_set_name(hbox_speed, "hbox_speed");
	gtk_widget_show(hbox_speed);
	gtk_box_pack_start(GTK_BOX(vbox_SelCD), hbox_speed, FALSE, FALSE, 0);
	GLADE_HOOKUP_OBJECT(select_cdrom_window, hbox_speed, "hbox_speed");
	
	// Speed label
	label_speed = gtk_label_new_with_mnemonic(
		"Manual CD-ROM speed selection.\n"
		"You may need to restart Gens for\n"
		"this setting to take effect."
		);
	gtk_widget_show(label_speed);
	gtk_box_pack_start(GTK_BOX(hbox_speed), label_speed, TRUE, TRUE, 0);
	GLADE_HOOKUP_OBJECT(select_cdrom_window, label_speed, "label_speed");
	
	// Add the dropdown for speed selection.
	combo_speed = gtk_combo_box_new_text();
	gtk_widget_set_name(combo_speed, "combo_speed");
	gtk_widget_set_size_request(combo_speed, 100, -1);
	for (i = 0; i < 14; i++)
	{
		if (CD_DriveSpeed[i] < 0)
			break;
		else if (CD_DriveSpeed[i] == 0)
			strcpy(tmp, "Auto");
		else
			sprintf(tmp, "%dx", CD_DriveSpeed[i]);
		gtk_combo_box_append_text(GTK_COMBO_BOX(combo_speed), tmp);
	}
	gtk_widget_show(combo_speed);
	gtk_box_pack_start(GTK_BOX(hbox_speed), combo_speed, FALSE, FALSE, 0);
	GLADE_HOOKUP_OBJECT(select_cdrom_window, combo_speed, "combo_speed");
	
	// Create an HButton Box for the buttons on the bottom.
	hbutton_box_bottomRow = gtk_hbutton_box_new();
	gtk_widget_set_name(hbutton_box_bottomRow, "hbutton_box_bottomRow");
	gtk_button_box_set_layout(GTK_BUTTON_BOX(hbutton_box_bottomRow), GTK_BUTTONBOX_END);
	gtk_widget_show(hbutton_box_bottomRow);
	gtk_box_pack_start(GTK_BOX(vbox_SelCD), hbutton_box_bottomRow, FALSE, FALSE, 0);
	GLADE_HOOKUP_OBJECT(select_cdrom_window, hbutton_box_bottomRow, "hbutton_box_bottomRow");
	
	// Cancel
	button_SelCD_Cancel = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	gtk_widget_set_name(button_SelCD_Cancel, "button_SelCD_Cancel");
	gtk_widget_show(button_SelCD_Cancel);
	gtk_box_pack_start(GTK_BOX(hbutton_box_bottomRow), button_SelCD_Cancel, FALSE, FALSE, 0);
	gtk_widget_add_accelerator(button_SelCD_Cancel, "activate", accel_group,
				   GDK_Escape, (GdkModifierType)(0), (GtkAccelFlags)(0));
	AddButtonCallback_Clicked(button_SelCD_Cancel, on_button_SelCD_Cancel_clicked);
	GLADE_HOOKUP_OBJECT(select_cdrom_window, button_SelCD_Cancel, "button_SelCD_Cancel");
	
	// Apply
	button_SelCD_Apply = gtk_button_new_from_stock(GTK_STOCK_APPLY);
	gtk_widget_set_name(button_SelCD_Apply, "button_SelCD_Apply");
	gtk_widget_show(button_SelCD_Apply);
	gtk_box_pack_start(GTK_BOX(hbutton_box_bottomRow), button_SelCD_Apply, FALSE, FALSE, 0);
	AddButtonCallback_Clicked(button_SelCD_Apply, on_button_SelCD_Apply_clicked);
	GLADE_HOOKUP_OBJECT(select_cdrom_window, button_SelCD_Apply, "button_SelCD_Apply");
	
	// Save
	button_SelCD_Save = gtk_button_new_from_stock(GTK_STOCK_SAVE);
	gtk_widget_set_name(button_SelCD_Save, "button_SelCD_Save");
	gtk_widget_show(button_SelCD_Save);
	gtk_box_pack_start(GTK_BOX(hbutton_box_bottomRow), button_SelCD_Save, FALSE, FALSE, 0);
	gtk_widget_add_accelerator(button_SelCD_Save, "activate", accel_group,
				   GDK_Return, (GdkModifierType)(0), (GtkAccelFlags)(0));
	gtk_widget_add_accelerator(button_SelCD_Save, "activate", accel_group,
				   GDK_KP_Enter, (GdkModifierType)(0), (GtkAccelFlags)(0));
	AddButtonCallback_Clicked(button_SelCD_Save, on_button_SelCD_Save_clicked);
	GLADE_HOOKUP_OBJECT(select_cdrom_window, button_SelCD_Save, "button_SelCD_Save");
	
	// Add the accel group.
	gtk_window_add_accel_group(GTK_WINDOW(select_cdrom_window), accel_group);
	
	return select_cdrom_window;
#endif
}


void Select_CDROM_Window_CreateChildWindows(HWND hWnd)
{
	HWND cdromDriveTitle;
	
	// CD-ROM Drive title
	cdromDriveTitle = CreateWindow(WC_STATIC, "CD-ROM Drive:",
				       WS_CHILD | WS_VISIBLE | SS_LEFT,
				       16, 8+2, 80, 12,
				       hWnd, NULL, ghInstance, NULL);
	SetWindowFont(cdromDriveTitle, fntMain, TRUE);
	
	// CD-ROM Drive dropdown box
	SelCD_cdromDropdownBox = CreateWindow(WC_COMBOBOX, NULL,
					      WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
					      16+80+8, 8, 288-80-16-16, 23,
					      hWnd, NULL, ghInstance, NULL);
	SetWindowFont(SelCD_cdromDropdownBox, fntMain, TRUE);
	
	// Populate the dropdown box.
	char cdromDriveStrings[128];
	char drvLetter[3] = {' ', ':', '\0'};
	char drvDropdownString[128];
	unsigned short cpos = 0;
	SHFILEINFO drvInfo;
	
	// Get all logical drive strings.
	GetLogicalDriveStrings(sizeof(cdromDriveStrings), cdromDriveStrings);
	
	while (cdromDriveStrings[cpos] && cpos < sizeof(cdromDriveStrings))
	{
		if (GetDriveType(&cdromDriveStrings[cpos]) == DRIVE_CDROM)
		{
			// CD-ROM drive.
			
			// Format the drive letter.
			drvLetter[0] = toupper(cdromDriveStrings[cpos]);
			
			// Create the dropdown string.
			strcpy(drvDropdownString, drvLetter);
			
			// Get drive information.
			SHGetFileInfo(&cdromDriveStrings[cpos], 0, &drvInfo, sizeof(drvInfo),
				      SHGFI_DISPLAYNAME | SHGFI_ICON | SHGFI_SMALLICON);
			
			// If a disc label is found, show it.
			if (drvInfo.szDisplayName[0])
			{
				strcat(drvDropdownString, " (");
				strcat(drvDropdownString, drvInfo.szDisplayName);
				strcat(drvDropdownString, ")");
			}
			
			// Add the drive to the dropdown.
			ComboBox_AddString(SelCD_cdromDropdownBox, drvDropdownString);
		}
		cpos += strlen(&cdromDriveStrings[cpos]) + 1;
	}
	
	// Buttons
	const int btnTop = 40;
	HWND btnOK, btnApply, btnCancel;
	
	btnOK = CreateWindow(WC_BUTTON, "&OK", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
			     18+8, btnTop, 75, 23,
			     hWnd, (HMENU)IDC_BTN_OK, ghInstance, NULL);
	SetWindowFont(btnOK, fntMain, TRUE);
	
	btnApply = CreateWindow(WC_BUTTON, "&Apply", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				18+8+75+8, btnTop, 75, 23,
				hWnd, (HMENU)IDC_BTN_APPLY, ghInstance, NULL);
	SetWindowFont(btnApply, fntMain, TRUE);
	
	btnCancel = CreateWindow(WC_BUTTON, "&Cancel", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				 18+8+75+8+75+8, btnTop, 75, 23,
				 hWnd, (HMENU)IDC_BTN_CANCEL, ghInstance, NULL);
	SetWindowFont(btnCancel, fntMain, TRUE);
}
