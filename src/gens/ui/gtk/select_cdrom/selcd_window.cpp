/***************************************************************************
 * Gens: (GTK+) Select CD-ROM Drive Window.                                *
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

#include "selcd_window.hpp"
#include "gens/gens_window.h"
#include "gens/gens_window_sync.hpp"
#include "ui/gens_ui.hpp"

// C includes.
#include <string.h>
#include <sys/stat.h>

// GTK+ includes.
#include <gtk/gtk.h>

// libgsft includes.
#include "libgsft/gsft_unused.h"
#include "libgsft/gsft_szprintf.h"
#include "libgsft/gsft_strlcpy.h"

// SegaCD
#include "emulator/g_mcd.hpp"
#include "segacd/cd_aspi.hpp"
#include "util/file/rom.hpp"
#include "gens_core/vdp/vdp_io.h"
#include "emulator/options.hpp"

// mingw doesn't provide S_ISLNK.
#ifndef S_ISLNK
#define S_ISLNK(x) (0)
#endif


// Window.
GtkWidget *selcd_window = NULL;

// Widgets.
static GtkWidget	*cboDeviceName;
static GtkWidget	*cboDriveSpeed;
static GtkWidget	*btnCancel, *btnApply, *btnSave;

// Select CD-ROM Drive load/save functions.
static void	selcd_window_init(void);
static int	selcd_window_save(void);

// Callbacks.
static gboolean	selcd_window_callback_close(GtkWidget *widget, GdkEvent *event, gpointer user_data);
static void	selcd_window_callback_response(GtkDialog *dialog, gint response_id, gpointer user_data);
static void	selcd_window_callback_combobox_changed(GtkComboBox *widget, gpointer user_data);

// Drive speed definitions. 0 == automatic; -1 == end of list.
static int CD_DriveSpeed[] = {0, 1, 2, 4, 8, 10, 12, 16, 20, 24, 32, 36, 40, 48, 50, 52, -1};


/**
 * selcd_window_show(): Show the Select CD-ROM Drive window.
 */
void selcd_window_show(void)
{
	if (selcd_window)
	{
		// Select CD-ROM Drive window is already visible. Set focus.
		gtk_widget_grab_focus(selcd_window);
		return;
	}
	
	// Create the window.
	selcd_window = gtk_dialog_new();
	gtk_container_set_border_width(GTK_CONTAINER(selcd_window), 4);
	gtk_window_set_title(GTK_WINDOW(selcd_window), "Select CD-ROM Drive");
	gtk_window_set_position(GTK_WINDOW(selcd_window), GTK_WIN_POS_CENTER);
	gtk_window_set_resizable(GTK_WINDOW(selcd_window), FALSE);
	gtk_window_set_type_hint(GTK_WINDOW(selcd_window), GDK_WINDOW_TYPE_HINT_DIALOG);
	gtk_dialog_set_has_separator(GTK_DIALOG(selcd_window), FALSE);
	
	// Callbacks for if the window is closed.
	g_signal_connect((gpointer)(selcd_window), "delete_event",
			 G_CALLBACK(selcd_window_callback_close), NULL);
	g_signal_connect((gpointer)(selcd_window), "destroy_event",
			 G_CALLBACK(selcd_window_callback_close), NULL);
	
	// Dialog response callback.
	g_signal_connect((gpointer)(selcd_window), "response",
			 G_CALLBACK(selcd_window_callback_response), NULL);
	
	// Get the dialog VBox.
	GtkWidget *vboxDialog = gtk_bin_get_child(GTK_BIN(selcd_window));
	gtk_box_set_spacing(GTK_BOX(vboxDialog), 4);
	gtk_widget_show(vboxDialog);
	
	// Add a label indicating typical values for the CD-ROM drive.
	GtkWidget *lblTypicalValues = gtk_label_new("Typical values are <i>/dev/cdrom</i>, <i>/dev/sr0</i>, or <i>/dev/hdc</i>");
	gtk_label_set_use_markup(GTK_LABEL(lblTypicalValues), true);
	gtk_widget_show(lblTypicalValues);
	gtk_box_pack_start(GTK_BOX(vboxDialog), lblTypicalValues, true, false, 0);
	
	// Create an HBox for the CD-ROM drive dropdown.
	GtkWidget *hboxDeviceName = gtk_hbox_new(false, 4);
	gtk_widget_show(hboxDeviceName);
	gtk_box_pack_start(GTK_BOX(vboxDialog), hboxDeviceName, true, false, 0);
	
	// Create the label for the device name dropdown.
	GtkWidget *lblDeviceName = gtk_label_new_with_mnemonic("CD-_ROM Drive:");
	gtk_widget_show(lblDeviceName);
	gtk_box_pack_start(GTK_BOX(hboxDeviceName), lblDeviceName, false, false, 0);
	
	// Create the dropdown for the CD-ROM device name.
	cboDeviceName = gtk_combo_box_entry_new_text();
	gtk_entry_set_max_length(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(cboDeviceName))), 63);
	gtk_label_set_mnemonic_widget(GTK_LABEL(lblDeviceName), cboDeviceName);
	gtk_widget_show(cboDeviceName);
	gtk_box_pack_start(GTK_BOX(hboxDeviceName), cboDeviceName, true, true, 0);
	g_signal_connect((gpointer)(cboDeviceName), "changed",
			 G_CALLBACK(selcd_window_callback_combobox_changed), NULL);

	// Create an HBox for CD-ROM drive speed selection.
	GtkWidget *hboxDriveSpeed = gtk_hbox_new(false, 4);
	gtk_widget_show(hboxDriveSpeed);
	gtk_box_pack_start(GTK_BOX(vboxDialog), hboxDriveSpeed, true, false, 0);
	
	// Create the label for the drive speed dropdown.
	GtkWidget *lblDriveSpeed = gtk_label_new_with_mnemonic(
			"Manual CD-ROM speed selection.\n"
			"You may need to restart Gens for\n"
			"this setting to take effect."
			);
	gtk_widget_show(lblDriveSpeed);
	gtk_box_pack_start(GTK_BOX(hboxDriveSpeed), lblDriveSpeed, false, false, 0);
	
	// Create a VBox for the CD-ROM drive speed selection dropdown.
	GtkWidget *vboxDriveSpeedDropdown = gtk_vbox_new(false, 0);
	gtk_widget_show(vboxDriveSpeedDropdown);
	gtk_box_pack_start(GTK_BOX(hboxDriveSpeed), vboxDriveSpeedDropdown, true, true, 0);
	
	// Create the dropdown for the CD-ROM drive speed selection.
	cboDriveSpeed = gtk_combo_box_new_text();
	char tmpSpeed[16];
	for (unsigned int i = 0; i < ((sizeof(CD_DriveSpeed) / sizeof(CD_DriveSpeed[0])) - 1); i++)
	{
		if (CD_DriveSpeed[i] < 0)
			break;
		else if (CD_DriveSpeed[i] == 0)
			strlcpy(tmpSpeed, "Auto", sizeof(tmpSpeed));
		else
			szprintf(tmpSpeed, sizeof(tmpSpeed), "%dx", CD_DriveSpeed[i]);

		gtk_combo_box_append_text(GTK_COMBO_BOX(cboDriveSpeed), tmpSpeed);
	}
	gtk_widget_show(cboDriveSpeed);
	gtk_box_pack_start(GTK_BOX(vboxDriveSpeedDropdown), cboDriveSpeed, true, false, 0);
	g_signal_connect((gpointer)(cboDriveSpeed), "changed",
			 G_CALLBACK(selcd_window_callback_combobox_changed), NULL);
	
	// Create the dialog buttons.
	btnApply  = gtk_dialog_add_button(GTK_DIALOG(selcd_window), GTK_STOCK_APPLY, GTK_RESPONSE_APPLY);
	btnCancel = gtk_dialog_add_button(GTK_DIALOG(selcd_window), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
	btnSave   = gtk_dialog_add_button(GTK_DIALOG(selcd_window), GTK_STOCK_SAVE, GTK_RESPONSE_OK);
	
#if (GTK_MAJOR_VERSION > 2) || ((GTK_MAJOR_VERSION == 2) && (GTK_MINOR_VERSION >= 6))
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(selcd_window),
						GTK_RESPONSE_OK,
						GTK_RESPONSE_CANCEL,
						GTK_RESPONSE_APPLY,
						-1);
#endif
	
	// Initialize the Select CD-ROM Drive window.
	selcd_window_init();
	
	// Set the window as transient to the main application window.
	gtk_window_set_transient_for(GTK_WINDOW(selcd_window), GTK_WINDOW(gens_window));
	
	// Show the window.
	gtk_widget_show_all(selcd_window);
}


/**
 * selcd_window_close(): Close the Select CD-ROM Drive window.
 */
void selcd_window_close(void)
{
	if (!selcd_window)
		return;
	
	// Destroy the window.
	gtk_widget_destroy(selcd_window);
	selcd_window = NULL;
}


/**
 * selcd_window_init(): Initialize the Select CD-ROM Drive window.
 */
static void selcd_window_init(void)
{
	// CD-ROM device name prefixes.
	static const char* cdrom_prefix[] = {"/dev/cdrom", "/dev/scd", "/dev/sr", NULL};
	
	// Set the CD-ROM drive text entry.
	gtk_entry_set_text(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(cboDeviceName))), cdromDeviceName);
	
	// Add all detected CD-ROM drives to the dropdown.
	// TODO: Improve this by using udev, hald, etc.
	char tmpDeviceName[128];
	struct stat fileStat;
	
	for (int curPrefix = 0; cdrom_prefix[curPrefix] != NULL; curPrefix++)
	{
		for (int i = -1; i <= 9; i++)
		{
			if (i == -1)
				strlcpy(tmpDeviceName, cdrom_prefix[curPrefix], sizeof(tmpDeviceName));
			else
				szprintf(tmpDeviceName, sizeof(tmpDeviceName), "%s%d", cdrom_prefix[curPrefix], i);
			
			if (lstat(tmpDeviceName, &fileStat))
			{
				// An error occurred while lstat()'ing this file. Skip it.
				continue;
			}
			
			// If this isn't "/dev/cdrom*", don't add it if it's a symlink.
			if (curPrefix != 0 && S_ISLNK(fileStat.st_mode))
				continue;
			
			// If this is neither a symlink nor a block device, don't add it.
			if (!S_ISLNK(fileStat.st_mode) && !S_ISBLK(fileStat.st_mode))
				continue;
			
			// Add the device file.
			gtk_combo_box_append_text(GTK_COMBO_BOX(cboDeviceName), tmpDeviceName);
		}
	}
	
	// CD-ROM drive speed.
	int driveSpeed = 0;
	for (unsigned int i = 0; i < ((sizeof(CD_DriveSpeed) / sizeof(CD_DriveSpeed[0])) - 1); i++)
	{
		if (CD_DriveSpeed[i] < 0)
			break;
		else if (CD_DriveSpeed[i] == cdromSpeed)
		{
			driveSpeed = i;
			break;
		}
	}
	gtk_combo_box_set_active(GTK_COMBO_BOX(cboDriveSpeed), driveSpeed);
	
	// Disable the "Apply" button initially.
	gtk_widget_set_sensitive(btnApply, false);
}


/**
 * selcd_window_save(): Save the CD-ROM device name.
 * @return 0 if the settings were saved; non-zero on error.
 */
static int selcd_window_save(void)
{
	bool restartASPI = false;
	bool restartCD = false;
	
	// Get the specified device name.
	const gchar *tmpDeviceName = gtk_entry_get_text(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(cboDeviceName))));
	
	if (tmpDeviceName && tmpDeviceName[0] != 0x00)
	{
		// CHeck if the device name was changed.
		if (strncmp(tmpDeviceName, cdromDeviceName, sizeof(cdromDeviceName)) != 0)
		{
			// Device name was changed.
			// This will cause a reset of the ASPI subsystem.
			if (SegaCD_Started && (CD_Load_System == CDROM_))
			{
				// ASPI is currently in use. Ask the user if they want to restart emulation.
				GensUI::MsgBox_Response response;
				response = GensUI::msgBox(
					"The CD-ROM drive is currently in use.\n"
					"Chanigng the CD-ROM drive will force the emulator to reset.\n"
					"Are you sure you want to do this?",
					"CD-ROM Drive in Use",
					GensUI::MSGBOX_ICON_WARNING | GensUI::MSGBOX_BUTTONS_YES_NO,
					selcd_window);
				
				if (response == GensUI::MSGBOX_RESPONSE_NO)
				{
					// Don't do anything.
					return 1;
				}
				
				// Stop SegaCD emulation.
				// TODO: Don't allow this if Netplay is enabled.
				restartCD = true;
				ROM::freeROM(Game);
			}
		}
		
		// Stop ASPI.
		ASPI_End();
		restartASPI = true;
		
		strlcpy(cdromDeviceName, tmpDeviceName, sizeof(cdromDeviceName));
	}
	
	// Drive speed.
	int driveSpeed = gtk_combo_box_get_active(GTK_COMBO_BOX(cboDriveSpeed));
	if (driveSpeed < 0 || driveSpeed >= ((sizeof(CD_DriveSpeed) / sizeof(CD_DriveSpeed[0])) - 1))
		driveSpeed = 0;
	cdromSpeed = CD_DriveSpeed[driveSpeed];
	
	if (restartASPI)
	{
		// Restart ASPI.
		ASPI_Init();
	}
	
	if (restartCD)
	{
		// Restart SegaCD enmulation.
		SegaCD_Started = Init_SegaCD(NULL);
		Options::setGameName();
		Sync_Gens_Window();
	}
	
	// Disable the "Apply" button.
	gtk_widget_set_sensitive(btnApply, false);
	
	// Settings saved.
	return 0;
}


/**
 * selcd_window_callback_close(): Close Window callback.
 * @param widget
 * @param event
 * @param user_data
 * @return FALSE to continue processing events; TRUE to stop processing events.
 */
static gboolean selcd_window_callback_close(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GSFT_UNUSED_PARAMETER(widget);
	GSFT_UNUSED_PARAMETER(event);
	GSFT_UNUSED_PARAMETER(user_data);
	
	selcd_window_close();
	return FALSE;
}


/**
 * selcd_window_callback_response(): Dialog Response callback.
 * @param dialog
 * @param response_id
 * @param user_data
 */
static void selcd_window_callback_response(GtkDialog *dialog, gint response_id, gpointer user_data)
{
	GSFT_UNUSED_PARAMETER(dialog);
	GSFT_UNUSED_PARAMETER(user_data);
	
	switch (response_id)
	{
		case GTK_RESPONSE_CANCEL:
			selcd_window_close();
			break;
		case GTK_RESPONSE_APPLY:
			selcd_window_save();
			break;
		case GTK_RESPONSE_OK:
			if (!selcd_window_save())
				selcd_window_close();
			break;
		
		case GTK_RESPONSE_DELETE_EVENT:
		default:
			// Other event. Don't do anything.
			// Also, don't do anything when the dialog is deleted.
			break;
	}
}


/**
 * selcd_window_callback_combobox_changed(): A combo box has been changed.
 * @param widget
 * @param user_data
 */
static void selcd_window_callback_combobox_changed(GtkComboBox *widget, gpointer user_data)
{
	GSFT_UNUSED_PARAMETER(widget);
	GSFT_UNUSED_PARAMETER(user_data);
	
	// Enable the "Apply" button.
	gtk_widget_set_sensitive(btnApply, true);
}
