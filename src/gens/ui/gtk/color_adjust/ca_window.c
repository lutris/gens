/***************************************************************************
 * Gens: (GTK+) Color Adjustment Window.                                   *
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

#include "ca_window.h"
#include "gens/gens_window.h"

// GTK+ includes.
#include <gtk/gtk.h>

// Unused Parameter macro.
#include "macros/unused.h"

// Gens includes.
#include "emulator/g_palette.h"
#include "emulator/g_main.hpp"
#include "emulator/g_update.hpp"
#include "gens_core/vdp/vdp_io.h"
#include "util/file/rom.hpp"

#ifdef GENS_DEBUGGER
#include "debugger/debugger.hpp"
#endif /* GENS_DEBUGGER */


// Window.
GtkWidget *ca_window = NULL;

// Widgets.
static GtkWidget	*sldContrast;
static GtkWidget	*sldBrightness;
static GtkWidget	*chkGrayscale;
static GtkWidget	*chkInverted;

// Color adjustment load/save functions.
static void	ca_window_init(void);
static void	ca_window_save(void);

// Callbacks.
static gboolean	ca_window_callback_close(GtkWidget *widget, GdkEvent *event, gpointer user_data);
static void	ca_window_callback_response(GtkDialog *dialog, gint response_id, gpointer user_data);


/**
 * ca_window_show(): Show the Color Adjustment window.
 */
void ca_window_show(void)
{
	if (ca_window)
	{
		// Color Adjustment window is already visible. Set focus.
		gtk_widget_grab_focus(ca_window);
		return;
	}
	
	// Create the window.
	ca_window = gtk_dialog_new();
	gtk_container_set_border_width(GTK_CONTAINER(ca_window), 4);
	gtk_window_set_title(GTK_WINDOW(ca_window), "Color Adjustment");
	gtk_window_set_position(GTK_WINDOW(ca_window), GTK_WIN_POS_CENTER);
	gtk_window_set_resizable(GTK_WINDOW(ca_window), FALSE);
	gtk_window_set_type_hint(GTK_WINDOW(ca_window), GDK_WINDOW_TYPE_HINT_DIALOG);
	gtk_dialog_set_has_separator(GTK_DIALOG(ca_window), FALSE);
	
	// Callbacks for if the window is closed.
	g_signal_connect((gpointer)(ca_window), "delete_event",
			 G_CALLBACK(ca_window_callback_close), NULL);
	g_signal_connect((gpointer)(ca_window), "destroy_event",
			 G_CALLBACK(ca_window_callback_close), NULL);
	
	// Dialog response callback.
	g_signal_connect((gpointer)(ca_window), "response",
			 G_CALLBACK(ca_window_callback_response), NULL);
	
	// Get the dialog VBox.
	GtkWidget *vboxDialog = GTK_DIALOG(ca_window)->vbox;
	gtk_widget_show(vboxDialog);
	
	// Create a table for the color adjustment sliders.
	GtkWidget *tblSliders = gtk_table_new(2, 2, FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(tblSliders), 4);
	gtk_table_set_col_spacings(GTK_TABLE(tblSliders), 4);
	gtk_widget_show(tblSliders);
	gtk_box_pack_start(GTK_BOX(vboxDialog), tblSliders, TRUE, TRUE, 0);
	
	// "Contrast" label.
	GtkWidget *lblContrast = gtk_label_new_with_mnemonic("Co_ntrast");
	gtk_misc_set_alignment(GTK_MISC(lblContrast), 0.0f, 0.5f);
	gtk_widget_show(lblContrast);
	gtk_table_attach(GTK_TABLE(tblSliders), lblContrast,
			 0, 1, 0, 1,
			 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
			 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 0, 0);
	
	// "Contrast" adjustment object.
	GtkObject *adjContrast = gtk_adjustment_new(0, -100, 100, 1, 0, 0);
	
	// "Contrast" slider.
	sldContrast = gtk_hscale_new(GTK_ADJUSTMENT(adjContrast));
	gtk_scale_set_value_pos(GTK_SCALE(sldContrast), GTK_POS_RIGHT);
	gtk_scale_set_digits(GTK_SCALE(sldContrast), 0);
	gtk_widget_set_size_request(sldContrast, 300, -1);
	gtk_widget_show(sldContrast);
	gtk_table_attach(GTK_TABLE(tblSliders), sldContrast,
			 1, 2, 0, 1,
			 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
			 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 0, 0);
	
	// Set the mnemonic widget for the "Contrast" label.
	gtk_label_set_mnemonic_widget(GTK_LABEL(lblContrast), sldContrast);
	
	// "Brightness" label.
	GtkWidget *lblBrightness = gtk_label_new_with_mnemonic("_Brightness");
	gtk_misc_set_alignment(GTK_MISC(lblBrightness), 0.0f, 0.5f);
	gtk_widget_show(lblBrightness);
	gtk_table_attach(GTK_TABLE(tblSliders), lblBrightness,
			 0, 1, 1, 2,
			 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
			 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 0, 0);
	
	// "Brightness" adjustment object.
	GtkObject *adjBrightness = gtk_adjustment_new(0, -100, 100, 1, 0, 0);
	
	// "Brightness" slider.
	sldBrightness = gtk_hscale_new(GTK_ADJUSTMENT(adjBrightness));
	gtk_scale_set_value_pos(GTK_SCALE(sldBrightness), GTK_POS_RIGHT);
	gtk_scale_set_digits(GTK_SCALE(sldBrightness), 0);
	gtk_widget_set_size_request(sldBrightness, 300, -1);
	gtk_widget_show(sldBrightness);
	gtk_table_attach(GTK_TABLE(tblSliders), sldBrightness,
			 1, 2, 1, 2,
			 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
			 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 0, 0);
	
	// Set the mnemonic widget for the "Brightness" label.
	gtk_label_set_mnemonic_widget(GTK_LABEL(lblBrightness), sldBrightness);
	
	// Create an HBox for some miscellaneous options.
	GtkWidget *hboxMiscOptions = gtk_hbox_new(FALSE, 4);
	gtk_widget_show(hboxMiscOptions);
	gtk_box_pack_start(GTK_BOX(vboxDialog), hboxMiscOptions, TRUE, FALSE, 0);
	
	// "Grayscale" checkbox.
	chkGrayscale = gtk_check_button_new_with_mnemonic("_Grayscale");
	gtk_widget_show(chkGrayscale);
	gtk_box_pack_start(GTK_BOX(hboxMiscOptions), chkGrayscale, TRUE, FALSE, 0);
	
	// "Invertd" checkbox.
	chkInverted = gtk_check_button_new_with_mnemonic("_Inverted");
	gtk_widget_show(chkInverted);
	gtk_box_pack_start(GTK_BOX(hboxMiscOptions), chkInverted, TRUE, FALSE, 0);
	
	// Create the dialog buttons.
	gtk_dialog_add_buttons(GTK_DIALOG(ca_window),
			       "gtk-cancel", GTK_RESPONSE_CANCEL,
			       "gtk-apply", GTK_RESPONSE_APPLY,
			       "gtk-save", GTK_RESPONSE_OK,
			       NULL);
#if (GTK_MAJOR_VERSION > 2) || ((GTK_MAJOR_VERSION == 2) && (GTK_MINOR_VERSION >= 6))
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(ca_window),
						GTK_RESPONSE_OK,
						GTK_RESPONSE_APPLY,
						GTK_RESPONSE_CANCEL,
						-1);
#endif
	
	// Initialize the color adjustment spinbuttons.
	ca_window_init();
	
	// Set the window as transient to the main application window.
	gtk_window_set_transient_for(GTK_WINDOW(ca_window), GTK_WINDOW(gens_window));
	
	// Show the window.
	gtk_widget_show_all(ca_window);
}


/**
 * ca_window_close(): Close the Color Adjustment window.
 */
void ca_window_close(void)
{
	if (!ca_window)
		return;
	
	// Destroy the window.
	gtk_widget_destroy(ca_window);
	ca_window = NULL;
}


/**
 * ca_window_init(): Initialize the Color Adjustment window widgets.
 */
static void ca_window_init(void)
{
	gtk_range_set_value(GTK_RANGE(sldContrast), (Contrast_Level - 100));
	gtk_range_set_value(GTK_RANGE(sldBrightness), (Brightness_Level - 100));
	
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chkGrayscale), Greyscale);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chkInverted), Invert_Color);
}


/**
 * ca_window_save(): Save the color adjustment settings.
 */
static void ca_window_save(void)
{
	Contrast_Level = gtk_range_get_value(GTK_RANGE(sldContrast)) + 100;
	Brightness_Level = gtk_range_get_value(GTK_RANGE(sldBrightness)) + 100;
	
	Greyscale = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chkGrayscale));
	Invert_Color = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chkInverted));
	
	// Recalculate palettes.
	Recalculate_Palettes();
	if (Game)
	{
		// Emulation is running. Update the CRAM.
		CRam_Flag = 1;
		
		if (!Paused)
		{
			// TODO: Just update CRAM. Don't update the frame.
			Update_Emulation_One();
			#ifdef GENS_DEBUGGER
				if (Debug)
					Update_Debug_Screen();
			#endif
		}
	}
}


/**
 * ca_window_callback_close(): Close Window callback.
 * @param widget
 * @param event
 * @param user_data
 * @return FALSE to continue processing events; TRUE to stop processing events.
 */
static gboolean ca_window_callback_close(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(widget);
	GENS_UNUSED_PARAMETER(event);
	GENS_UNUSED_PARAMETER(user_data);
	
	ca_window_close();
	return FALSE;
}


/**
 * ca_window_callback_response(): Dialog Response callback.
 * @param dialog
 * @param response_id
 * @param user_data
 */
static void ca_window_callback_response(GtkDialog *dialog, gint response_id, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(dialog);
	GENS_UNUSED_PARAMETER(user_data);
	
	switch (response_id)
	{
		case GTK_RESPONSE_CANCEL:
			ca_window_close();
			break;
		case GTK_RESPONSE_APPLY:
			ca_window_save();
			break;
		case GTK_RESPONSE_OK:
			ca_window_save();
			ca_window_close();
			break;
		
		case GTK_RESPONSE_DELETE_EVENT:
		default:
			// Other event. Don't do anything.
			// Also, don't do anything when the dialog is deleted.
			break;
	}
}
