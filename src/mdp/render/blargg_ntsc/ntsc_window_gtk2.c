/***************************************************************************
 * MDP: Blargg's NTSC Filter. (Window Code) (GTK+)                         *
 *                                                                         *
 * Copyright (c) 2006 by Shay Green                                        *
 * MDP version Copyright (c) 2008-2009 by David Korth                      *
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

// GTK+ includes.
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

// C includes.
#include <math.h>
#include <stdio.h>

#include "ntsc_window.h"
#include "ntsc_window_common.h"

#include "mdp_render_blargg_ntsc.h"
#include "mdp_render_blargg_ntsc_plugin.h"
#include "md_ntsc.hpp"

// MDP error codes.
#include "mdp/mdp_error.h"

// libgsft includes.
#include "libgsft/gsft_szprintf.h"

// Window.
static GtkWidget *ntsc_window = NULL;

// Widgets.
// TODO
static GtkWidget *cboPresets;
static GtkWidget *chkScanline;
static GtkWidget *chkInterp;
static GtkWidget *lblCtrlValues[NTSC_CTRL_COUNT];
static GtkWidget *hscCtrlValues[NTSC_CTRL_COUNT];
static GtkWidget *chkCXA2025AS;

// Callbacks.
static gboolean	ntsc_window_callback_close(GtkWidget *widget, GdkEvent *event, gpointer user_data);
static void	ntsc_window_callback_response(GtkDialog *dialog, gint response_id, gpointer user_data);
static void	ntsc_window_callback_cboPresets_changed(GtkComboBox *widget, gpointer user_data);
static void	ntsc_window_callback_hscCtrlValues_value_changed(GtkRange *range, gpointer user_data);
static void	ntsc_window_callback_chkScanline_toggled(GtkToggleButton *togglebutton, gpointer user_data);
static void	ntsc_window_callback_chkInterp_toggled(GtkToggleButton *togglebutton, gpointer user_data);
static void	ntsc_window_callback_chkCXA2025AS_toggled(GtkToggleButton *togglebutton, gpointer user_data);

static gboolean	ntsc_window_do_callbacks;


/**
 * ntsc_window_show(): Show the NTSC Plugin Options window.
 * @param parent Parent window.
 */
void ntsc_window_show(void *parent)
{
	if (ntsc_window)
	{
		// NTSC Plugin Options window is already visible.
		// Set focus.
		gtk_widget_grab_focus(ntsc_window);
		return;
	}
	
	// Don't do any callbacks yet.
	ntsc_window_do_callbacks = FALSE;
	
	// Create the NTSC Plugin Options window.
	ntsc_window = gtk_dialog_new();
	gtk_container_set_border_width(GTK_CONTAINER(ntsc_window), 4);
	gtk_window_set_title(GTK_WINDOW(ntsc_window), "Blargg's NTSC Filter");
	gtk_window_set_position(GTK_WINDOW(ntsc_window), GTK_WIN_POS_CENTER);
	gtk_window_set_resizable(GTK_WINDOW(ntsc_window), FALSE);
	gtk_window_set_type_hint(GTK_WINDOW(ntsc_window), GDK_WINDOW_TYPE_HINT_DIALOG);
	gtk_dialog_set_has_separator(GTK_DIALOG(ntsc_window), FALSE);
	
	// Callbacks for if the window is closed.
	g_signal_connect((gpointer)ntsc_window, "delete_event",
			 G_CALLBACK(ntsc_window_callback_close), NULL);
	g_signal_connect((gpointer)ntsc_window, "destroy_event",
			 G_CALLBACK(ntsc_window_callback_close), NULL);
	
	// Dialog response callback.
	g_signal_connect((gpointer)ntsc_window, "response",
			 G_CALLBACK(ntsc_window_callback_response), NULL);
	
	// Get the dialog VBox.
	GtkWidget *vboxDialog = gtk_bin_get_child(GTK_BIN(ntsc_window));
	gtk_widget_show(vboxDialog);
	
	// Create the main VBox.
	GtkWidget *vboxMain = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vboxMain);
	gtk_container_add(GTK_CONTAINER(vboxDialog), vboxMain);
	
	// Create the main frame.
	GtkWidget *fraMain = gtk_frame_new("NTSC Configuration");
	gtk_widget_show(fraMain);
	gtk_box_pack_start(GTK_BOX(vboxMain), fraMain, FALSE, TRUE, 0);
	gtk_frame_set_shadow_type(GTK_FRAME(fraMain), GTK_SHADOW_ETCHED_IN);
	
	// Create the VBox for the frame.
	GtkWidget *vboxFrame = gtk_vbox_new(FALSE, 8);
	gtk_container_set_border_width(GTK_CONTAINER(vboxFrame), 8);
	gtk_widget_show(vboxFrame);
	gtk_container_add(GTK_CONTAINER(fraMain), vboxFrame);
	
	// Add an HBox for the presets dropdown.
	GtkWidget *hboxPresets = gtk_hbox_new(FALSE, 8);
	gtk_widget_show(hboxPresets);
	gtk_box_pack_start(GTK_BOX(vboxFrame), hboxPresets, FALSE, FALSE, 0);
	
	// Add a label for the presets dropdown.
	GtkWidget *lblPresets = gtk_label_new_with_mnemonic("_Presets:");
	gtk_widget_show(lblPresets);
	gtk_box_pack_start(GTK_BOX(hboxPresets), lblPresets, FALSE, FALSE, 0);
	
	// Add the presets dropdown.
	cboPresets = gtk_combo_box_new_text();
	gtk_widget_show(cboPresets);
	gtk_box_pack_start(GTK_BOX(hboxPresets), cboPresets, FALSE, FALSE, 0);
	gtk_label_set_mnemonic_widget(GTK_LABEL(lblPresets), cboPresets);
	g_signal_connect((gpointer)cboPresets, "changed",
			 G_CALLBACK(ntsc_window_callback_cboPresets_changed), NULL);
	
	// Add the presets to the dropdown.
	int i;
	for (i = 0; i < NTSC_PRESETS_COUNT; i++)
	{
		gtk_combo_box_append_text(GTK_COMBO_BOX(cboPresets), ntsc_presets[i].name);
	}
	
	// Scanlines checkbox.
	chkScanline = gtk_check_button_new_with_mnemonic("S_canlines");
	gtk_widget_show(chkScanline);
	gtk_box_pack_start(GTK_BOX(hboxPresets), chkScanline, FALSE, FALSE, 0);
	g_signal_connect((gpointer)chkScanline, "toggled",
			 G_CALLBACK(ntsc_window_callback_chkScanline_toggled), NULL);
	
	// Interpolation checkbox.
	chkInterp = gtk_check_button_new_with_mnemonic("_Interpolation");
	gtk_widget_show(chkInterp);
	gtk_box_pack_start(GTK_BOX(hboxPresets), chkInterp, FALSE, FALSE, 0);
	g_signal_connect((gpointer)chkInterp, "toggled",
			 G_CALLBACK(ntsc_window_callback_chkInterp_toggled), NULL);
	
	// Sony CXA2025AS US decoder checkbox.
	chkCXA2025AS = gtk_check_button_new_with_mnemonic("Use Sony C_XA2025AS US decoder.");
	gtk_widget_show(chkCXA2025AS);
	gtk_box_pack_start(GTK_BOX(vboxFrame), chkCXA2025AS, FALSE, FALSE, 0);
	g_signal_connect((gpointer)chkCXA2025AS, "toggled",
			 G_CALLBACK(ntsc_window_callback_chkCXA2025AS_toggled), NULL);
	
	// Create a table for the adjustment widgets.
	// First column: Name
	// Second column: Widget
	GtkWidget *tblWidgets = gtk_table_new(NTSC_CTRL_COUNT, 3, FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(tblWidgets), 4);
	gtk_table_set_col_spacings(GTK_TABLE(tblWidgets), 8);
	gtk_box_pack_start(GTK_BOX(vboxFrame), tblWidgets, TRUE, TRUE, 0);
	
	// Create the widgets.
	for (i = 0; i < NTSC_CTRL_COUNT; i++)
	{
		// Label alignment.
		GtkWidget *alignWidgetName = gtk_alignment_new(0.0f, 0.5f, 0, 0);
		gtk_widget_show(alignWidgetName);
		gtk_table_attach(GTK_TABLE(tblWidgets), alignWidgetName,
				 0, 1, i, i + 1,
				 (GtkAttachOptions)(GTK_FILL),
				 (GtkAttachOptions)(0), 0, 0);
		
		// Label.
		GtkWidget *lblWidgetName = gtk_label_new_with_mnemonic(ntsc_controls[i].name_mnemonic);
		gtk_widget_show(lblWidgetName);
		gtk_container_add(GTK_CONTAINER(alignWidgetName), lblWidgetName);
		
		// Value Label alignment.
		GtkWidget *alignCtrlValue = gtk_alignment_new(1.0f, 0.5f, 0, 0);
		gtk_widget_set_size_request(alignCtrlValue, 40, -1);
		gtk_widget_show(alignWidgetName);
		gtk_table_attach(GTK_TABLE(tblWidgets), alignCtrlValue,
				 1, 2, i, i + 1,
				 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
				 (GtkAttachOptions)(0), 0, 0);
		
		// Value Label.
		lblCtrlValues[i] = gtk_label_new(NULL);
		gtk_widget_show(lblCtrlValues[i]);
		gtk_container_add(GTK_CONTAINER(alignCtrlValue), lblCtrlValues[i]);
		
		// Adjustment object.
		GtkObject *adjWidget = gtk_adjustment_new(0, ntsc_controls[i].min,
							  ntsc_controls[i].max,
							  ntsc_controls[i].step,
							  ntsc_controls[i].step * 2, 0);
		
		// GtkHScale.
		hscCtrlValues[i] = gtk_hscale_new(GTK_ADJUSTMENT(adjWidget));
		gtk_scale_set_draw_value(GTK_SCALE(hscCtrlValues[i]), FALSE);
		gtk_widget_set_size_request(hscCtrlValues[i], 256, -1);
		gtk_widget_show(hscCtrlValues[i]);
		gtk_label_set_mnemonic_widget(GTK_LABEL(lblWidgetName), hscCtrlValues[i]);
		gtk_table_attach(GTK_TABLE(tblWidgets), hscCtrlValues[i],
				 2, 3, i, i + 1,
				 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
				 (GtkAttachOptions)(0), 0, 0);
		g_signal_connect((gpointer)hscCtrlValues[i], "value-changed",
				 G_CALLBACK(ntsc_window_callback_hscCtrlValues_value_changed),
				 GINT_TO_POINTER(i));
		
		// Initialize the value label.
		ntsc_window_callback_hscCtrlValues_value_changed(GTK_RANGE(hscCtrlValues[i]), GINT_TO_POINTER(i));
	}
	
	// Create the dialog buttons.
	
	// "Close" button.
	gtk_dialog_add_button(GTK_DIALOG(ntsc_window),
			      GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE);
	
	// Set the window as modal to the main application window.
	if (parent)
		gtk_window_set_transient_for(GTK_WINDOW(ntsc_window), GTK_WINDOW(parent));
	
	// Load the current settings.
	ntsc_window_load_settings();
	
	// Show the window.
	gtk_widget_show_all(ntsc_window);
	
	// Register the window with MDP Host Services.
	ntsc_host_srv->window_register(&mdp, ntsc_window);
}


/**
 * ntsc_window_close(): Close the VDP Layer Options window.
 */
void ntsc_window_close(void)
{
	if (!ntsc_window)
		return;
	
	// Unregister the window from MDP Host Services.
	ntsc_host_srv->window_unregister(&mdp, ntsc_window);
	
	// Destroy the window.
	gtk_widget_destroy(ntsc_window);
	ntsc_window = NULL;
}


/**
 * ntsc_window_callback_close(): Close Window callback.
 * @param widget
 * @param event
 * @param user_data
 * @return FALSE to continue processing events; TRUE to stop processing events.
 */
static gboolean ntsc_window_callback_close(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	MDP_UNUSED_PARAMETER(widget);
	MDP_UNUSED_PARAMETER(event);
	MDP_UNUSED_PARAMETER(user_data);
	
	ntsc_window_close();
	return FALSE;
}


/**
 * ntsc_window_callback_response(): Dialog Response callback.
 * @param dialog
 * @param response_id
 * @param user_data
 */
static void ntsc_window_callback_response(GtkDialog *dialog, gint response_id, gpointer user_data)
{
	MDP_UNUSED_PARAMETER(dialog);
	MDP_UNUSED_PARAMETER(user_data);
	
	switch (response_id)
	{
		case GTK_RESPONSE_CLOSE:
			// Close.
			ntsc_window_close();
			break;
		
		default:
			// Unknown response ID.
			break;
	}
}


/**
 * ntsc_window_load_settings(): Load the NTSC settings.
 */
void MDP_FNCALL ntsc_window_load_settings(void)
{
	if (!ntsc_window)
		return;
	
	ntsc_window_do_callbacks = FALSE;
	
	// Set the preset dropdown box.
	int i;
	for (i = 0; i < NTSC_PRESETS_COUNT; i++)
	{
		if (!ntsc_presets[i].setup)
		{
			// "Custom". This is the last item in the predefined list.
			// Since the current setup doesn't match anything else,
			// it must be a custom setup.
			gtk_combo_box_set_active(GTK_COMBO_BOX(cboPresets), i);
			break;
		}
		else
		{
			// Check if this preset matches the current setup.
			if (!memcmp(mdp_md_ntsc_setup.params,
				    ntsc_presets[i].setup->params,
				    sizeof(mdp_md_ntsc_setup.params)))
			{
				// Match found!
				gtk_combo_box_set_active(GTK_COMBO_BOX(cboPresets), i);
				break;
			}
		}
	}
	
	// Scanlines / Interpolation / CXA2025AS
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chkScanline), (mdp_md_ntsc_effects & MDP_MD_NTSC_EFFECT_SCANLINE));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chkInterp), (mdp_md_ntsc_effects & MDP_MD_NTSC_EFFECT_INTERP));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chkCXA2025AS), (mdp_md_ntsc_effects & MDP_MD_NTSC_EFFECT_CXA2025AS));
	
	// Load all settings.
	for (i = 0; i < NTSC_CTRL_COUNT; i++)
	{
		gtk_range_set_value(GTK_RANGE(hscCtrlValues[i]),
				    ntsc_internal_to_display(i, mdp_md_ntsc_setup.params[i]));
	}
	
	ntsc_window_do_callbacks = TRUE;
}


/**
 * ntsc_window_callback_cboPresets_changed(): The "Presets" combo box has been changed.
 * @param widget
 * @param user_data
 */
static void ntsc_window_callback_cboPresets_changed(GtkComboBox *widget, gpointer user_data)
{
	MDP_UNUSED_PARAMETER(user_data);
	
	if (!ntsc_window_do_callbacks)
		return;
	
	// Load the specified preset setup.
	int i = gtk_combo_box_get_active(widget);
	if (i == -1 || i >= NTSC_PRESETS_COUNT)
		return;
	
	if (!ntsc_presets[i].setup)
		return;
	
	// Reinitialize the NTSC filter with the new settings.
	mdp_md_ntsc_setup = *(ntsc_presets[i].setup);
	mdp_md_ntsc_reinit_setup();
	
	// Load the new settings in the window.
	ntsc_window_load_settings();
}


/**
 * ntsc_window_callback_hscCtrlValues_value_changed(): One of the adjustment controls has been changed.
 * @param range
 * @param user_data
 */
static void ntsc_window_callback_hscCtrlValues_value_changed(GtkRange *range, gpointer user_data)
{
	int i = GPOINTER_TO_INT(user_data);
	if (i < 0 || i >= NTSC_CTRL_COUNT)
		return;
	
	// Update the label for the adjustment widget.
	char tmp[16];
	int val = (int)rint(gtk_range_get_value(range));
	
	// Adjust the value to have the appropriate number of decimal places.
	if (i == 0)
	{
		// Hue. No decimal places.
		szprintf(tmp, sizeof(tmp), "%d" NTSC_DEGREE_SYMBOL, val);
	}
	else
	{
		// Other adjustment. 2 decimal places.
		szprintf(tmp, sizeof(tmp), "%0.2f", ((double)val / 100.0));
	}
	
	gtk_label_set_text(GTK_LABEL(lblCtrlValues[i]), tmp);
	
	if (!ntsc_window_do_callbacks)
		return;
	
	// Adjust the NTSC filter.
	mdp_md_ntsc_setup.params[i] = ntsc_display_to_internal(i, val);
	
	// Set the "Presets" dropdown to "Custom".
	gtk_combo_box_set_active(GTK_COMBO_BOX(cboPresets), NTSC_PRESETS_COUNT-1);
	
	// Reinitialize the NTSC filter with the new settings.
	mdp_md_ntsc_reinit_setup();
}


/**
 * ntsc_window_callback_chkScanline_toggled(): The "Scanlines" checkbox was toggled.
 * @param togglebutton
 * @param user_data
 */
static void ntsc_window_callback_chkScanline_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
	MDP_UNUSED_PARAMETER(user_data);
	
	if (!ntsc_window_do_callbacks)
		return;
	
	if (gtk_toggle_button_get_active(togglebutton))
		mdp_md_ntsc_effects |= MDP_MD_NTSC_EFFECT_SCANLINE;
	else
		mdp_md_ntsc_effects &= ~MDP_MD_NTSC_EFFECT_SCANLINE;
}


/**
 * ntsc_window_callback_chkScanline_toggled(): The "Interpolation" checkbox was toggled.
 * @param togglebutton
 * @param user_data
 */
static void ntsc_window_callback_chkInterp_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
	MDP_UNUSED_PARAMETER(user_data);
	
	if (!ntsc_window_do_callbacks)
		return;
	
	if (gtk_toggle_button_get_active(togglebutton))
		mdp_md_ntsc_effects |= MDP_MD_NTSC_EFFECT_INTERP;
	else
		mdp_md_ntsc_effects &= ~MDP_MD_NTSC_EFFECT_INTERP;
}


/**
 * ntsc_window_callback_chkCXA2025AS_toggled(): The "Use Sony CXA2025AS US decoder." checkbox was toggled.
 * @param togglebutton
 * @param user_data
 */
static void ntsc_window_callback_chkCXA2025AS_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
	MDP_UNUSED_PARAMETER(user_data);
	
	if (!ntsc_window_do_callbacks)
		return;
	
	if (gtk_toggle_button_get_active(togglebutton))
		mdp_md_ntsc_effects |= MDP_MD_NTSC_EFFECT_CXA2025AS;
	else
		mdp_md_ntsc_effects &= ~MDP_MD_NTSC_EFFECT_CXA2025AS;
	
	// Reinitialize the NTSC filter with the new settings.
	mdp_md_ntsc_reinit_setup();
}
