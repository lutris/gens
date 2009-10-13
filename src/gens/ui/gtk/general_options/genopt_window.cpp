/***************************************************************************
 * Gens: (GTK+) General Options Window.                                    *
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

// GTK+ includes.
#include <gtk/gtk.h>

// Main settings.
#include "emulator/g_main.hpp"
#include "emulator/options.hpp"

// libgsft includes.
#include "libgsft/gsft_unused.h"
#include "libgsft/gsft_szprintf.h"

// Video Drawing.
#include "video/vdraw.h"


typedef struct _gtk_color_t
{
	const char name[8];
	const char color_normal[8];
	const char color_active[8];
	const char color_prelight[8];
	const char color_selected[8];
	const char color_insensitive[8];
} gtk_color_t;

// On-Screen Display colors.
static const gtk_color_t genopt_colors_OSD[4] =
{
	{"white", "#FFFFFF", "#E0E0E0", "#FFFFFF", "#FFFFFF", "#C0C0C0"},
	{"blue",  "#0000FF", "#0000E0", "#8080FF", "#0000FF", "#1C1C1C"},
	{"green", "#00FF00", "#00E000", "#80FF80", "#00FF00", "#979797"},
	{"red",   "#FF0000", "#E00000", "#FF8080", "#FF0000", "#4C4C4C"},
};

// Intro effect colors.
static const gtk_color_t genopt_colors_IntroEffect[8] =
{
	{"black",  "#000000", "#000000", "#808080", "#000000", "#808080"},
	{"blue",   "#0000FF", "#0000E0", "#8080FF", "#0000FF", "#202020"},
	{"green",  "#00FF00", "#00E000", "#80FF80", "#00FF00", "#989898"},
	{"cyan",   "#00FFFF", "#00E0E0", "#80FFFF", "#00FFFF", "#B3B3B3"},
	{"red",    "#FF0000", "#E00000", "#FF8080", "#FF0000", "#4C4C4C"},
	{"purple", "#FF00FF", "#E000E0", "#FF80FF", "#FF00FF", "#686868"},
	{"yellow", "#FFFF00", "#E0E000", "#FFFF80", "#FFFF00", "#E3E3E3"},
	{"white",  "#FFFFFF", "#E0E0E0", "#FFFFFF", "#FFFFFF", "#C0C0C0"},
};


// Window.
GtkWidget *genopt_window = NULL;

// Widgets.
static GtkWidget	*btnCancel, *btnApply, *btnSave;

// Widgets: On-Screen Display.
static GtkWidget	*chkOSD_Enable[2];
static GtkWidget	*chkOSD_DoubleSized[2];
static GtkWidget	*chkOSD_Transparency[2];
static GtkWidget	*optOSD_Color[2][4];

// Widgets: Intro Effect.
static GtkWidget	*cboIntroEffect;
static GtkWidget	*optIntroEffectColor[8];

// Widgets: Miscellaneous settings.
static GtkWidget	*chkMisc_AutoFixChecksum;
static GtkWidget	*chkMisc_AutoPause;
static GtkWidget	*chkMisc_FastBlur;
static GtkWidget	*chkMisc_SegaCDLEDs;
static GtkWidget	*chkMisc_BorderColorEmulation;
static GtkWidget	*chkMisc_PauseTint;

// Widget creation functions.
static void	genopt_window_create_osd_frame(GtkWidget *container, const char* title, int index);
static void	genopt_window_create_color_radio_buttons(GtkWidget* container,
							 const char* title,
							 const gtk_color_t *colors,
							 int buttonSet);
static void	genopt_window_create_misc_settings_frame(GtkWidget *container);

// Configuration load/save functions.
static void	genopt_window_init(void);
static void	genopt_window_save(void);

// Callbacks.
static gboolean	genopt_window_callback_close(GtkWidget *widget, GdkEvent *event, gpointer user_data);
static void	genopt_window_callback_response(GtkDialog *dialog, gint response_id, gpointer user_data);
static void	genopt_window_callback_widget_changed(void);


/**
 * genopt_window_show(): Show the General Options window.
 */
void genopt_window_show(void)
{
	if (genopt_window)
	{
		// Controller Configuration window is already visible. Set focus.
		gtk_widget_grab_focus(genopt_window);
		return;
	}
	
	genopt_window = gtk_dialog_new();
	gtk_container_set_border_width(GTK_CONTAINER(genopt_window), 0);
	gtk_window_set_title(GTK_WINDOW(genopt_window), "General Options");
	gtk_window_set_position(GTK_WINDOW(genopt_window), GTK_WIN_POS_CENTER);
	gtk_window_set_resizable(GTK_WINDOW(genopt_window), false);
	gtk_window_set_type_hint(GTK_WINDOW(genopt_window), GDK_WINDOW_TYPE_HINT_DIALOG);
	gtk_dialog_set_has_separator(GTK_DIALOG(genopt_window), false);
	
	// Callbacks for if the window is closed.
	g_signal_connect((gpointer)genopt_window, "delete_event",
			  G_CALLBACK(genopt_window_callback_close), NULL);
	g_signal_connect((gpointer)genopt_window, "destroy_event",
			  G_CALLBACK(genopt_window_callback_close), NULL);
	
	// Dialog response callback.
	g_signal_connect((gpointer)genopt_window, "response",
			 G_CALLBACK(genopt_window_callback_response), NULL);
	
	// Get the dialog VBox.
	GtkWidget *vboxDialog = gtk_bin_get_child(GTK_BIN(genopt_window));
	gtk_widget_show(vboxDialog);
	
	// Create the main HBox.
	GtkWidget *hboxMain = gtk_hbox_new(false, 0);
	gtk_widget_show(hboxMain);
	gtk_box_pack_start(GTK_BOX(vboxDialog), hboxMain, true, true, 0);
	
	// On-screen display frame.
	GtkWidget *fraOSDMain = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(fraOSDMain), GTK_SHADOW_ETCHED_IN);
	gtk_container_set_border_width(GTK_CONTAINER(fraOSDMain), 4);
	gtk_widget_show(fraOSDMain);
	gtk_box_pack_start(GTK_BOX(hboxMain), fraOSDMain, true, true, 0);
	
	// Label for the on-screen display frame.
	GtkWidget *lblOSDMain = gtk_label_new("<b><i>On-Screen Display</i></b>");
	gtk_label_set_use_markup(GTK_LABEL(lblOSDMain), true);
	gtk_widget_show(lblOSDMain);
	gtk_frame_set_label_widget(GTK_FRAME(fraOSDMain), lblOSDMain);
	
	// VBox for the on-screen display frame.
	GtkWidget *vboxOSDMain = gtk_vbox_new(false, 0);
	gtk_widget_show(vboxOSDMain);
	gtk_container_add(GTK_CONTAINER(fraOSDMain), vboxOSDMain);
	
	// Add FPS and Message frames to the OSD frame.
	genopt_window_create_osd_frame(vboxOSDMain, "FPS counter", 0);
	genopt_window_create_osd_frame(vboxOSDMain, "Message", 1);
	
	// Create the Miscellaneous Settings frame.
	genopt_window_create_misc_settings_frame(hboxMain);
	
	// Create the dialog buttons.
	btnApply  = gtk_dialog_add_button(GTK_DIALOG(genopt_window), GTK_STOCK_APPLY, GTK_RESPONSE_APPLY);
	btnCancel = gtk_dialog_add_button(GTK_DIALOG(genopt_window), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
	btnSave   = gtk_dialog_add_button(GTK_DIALOG(genopt_window), GTK_STOCK_SAVE, GTK_RESPONSE_OK);
	
#if (GTK_MAJOR_VERSION > 2) || ((GTK_MAJOR_VERSION == 2) && (GTK_MINOR_VERSION >= 6))
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(genopt_window),
						GTK_RESPONSE_OK,
						GTK_RESPONSE_CANCEL,
						GTK_RESPONSE_APPLY,
						-1);
#endif
	
	// Initialize the internal data variables.
	genopt_window_init();
	
	// Set the window as transient to the main application window.
	gtk_window_set_transient_for(GTK_WINDOW(genopt_window), GTK_WINDOW(gens_window));
	
	// Show the window.
	gtk_widget_show_all(genopt_window);
}


/**
 * genopt_window_create_osd_frame(): Creates an on-screen display frame.
 * @param container Container for the frame.
 * @param title Title for this frame.
 * @param index Frame index.
 */
static void genopt_window_create_osd_frame(GtkWidget *container, const char* title, const int index)
{
	char tmp[64];
	
	// Create the frame.
	szprintf(tmp, sizeof(tmp), "<b><i>%s</i></b>", title);
	GtkWidget *fraOSD = gtk_frame_new(tmp);
	gtk_frame_set_shadow_type(GTK_FRAME(fraOSD), GTK_SHADOW_ETCHED_IN);
	gtk_label_set_use_markup(GTK_LABEL(gtk_frame_get_label_widget(GTK_FRAME(fraOSD))), true);
	gtk_container_set_border_width(GTK_CONTAINER(fraOSD), 4);
	gtk_widget_show(fraOSD);
	gtk_box_pack_start(GTK_BOX(container), fraOSD, true, true, 0);
	
	// Create the vbox for the frame.
	GtkWidget *vboxOSD = gtk_vbox_new(false, 0);
	gtk_widget_show(vboxOSD);
	gtk_container_add(GTK_CONTAINER(fraOSD), vboxOSD);
	
	// Create the checkboxes.
	
	// Enable
	chkOSD_Enable[index] = gtk_check_button_new_with_label("Enable");
	gtk_container_set_border_width(GTK_CONTAINER(chkOSD_Enable[index]), 2);
	gtk_widget_show(chkOSD_Enable[index]);
	gtk_box_pack_start(GTK_BOX(vboxOSD), chkOSD_Enable[index], false, false, 0);
	g_signal_connect(GTK_OBJECT(chkOSD_Enable[index]), "toggled",
			 G_CALLBACK(genopt_window_callback_widget_changed), NULL);
	
	// Double Sized
	chkOSD_DoubleSized[index] = gtk_check_button_new_with_label("Double Sized");
	gtk_container_set_border_width(GTK_CONTAINER(chkOSD_DoubleSized[index]), 2);
	gtk_widget_show(chkOSD_DoubleSized[index]);
	gtk_box_pack_start(GTK_BOX(vboxOSD), chkOSD_DoubleSized[index], false, false, 0);
	g_signal_connect(GTK_OBJECT(chkOSD_DoubleSized[index]), "toggled",
			 G_CALLBACK(genopt_window_callback_widget_changed), NULL);
	
	// Transparency
	chkOSD_Transparency[index] = gtk_check_button_new_with_label("Transparency");
	gtk_container_set_border_width(GTK_CONTAINER(chkOSD_Transparency[index]), 2);
	gtk_widget_show(chkOSD_Transparency[index]);
	gtk_box_pack_start(GTK_BOX(vboxOSD), chkOSD_Transparency[index], false, false, 0);
	g_signal_connect(GTK_OBJECT(chkOSD_Transparency[index]), "toggled",
			 G_CALLBACK(genopt_window_callback_widget_changed), NULL);
	
	// Color HBox
	GtkWidget *hboxOSD_Color = gtk_hbox_new(false, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hboxOSD_Color), 8);
	gtk_widget_show(hboxOSD_Color);
	gtk_container_add(GTK_CONTAINER(vboxOSD), hboxOSD_Color);
	
	// Color Radio Buttons
	genopt_window_create_color_radio_buttons(hboxOSD_Color, "Color:", genopt_colors_OSD, index);
}


/**
 * genopt_window_create_color_radio_buttons(): Create color radio buttons.
 * @param container Container for the radio buttons.
 * @param title Title for this color button group.
 * @param colors Array of colors.
 * @param buttonSet Button set ID.
 */
static void genopt_window_create_color_radio_buttons(GtkWidget* container,
						     const char* title,
						     const gtk_color_t *colors,
						     int buttonSet)
{
	int numColors = 0;
	
	// Make sure a valid button set is specified.
	if (colors == genopt_colors_OSD)
	{
		if (buttonSet < 0 || buttonSet > 1)
			return;
		numColors = 4;
	}
	else if (colors == genopt_colors_IntroEffect)
	{
		if (buttonSet != 0)
			return;
		numColors = 8;
	}
	else
	{
		return;
	}
	
	// Color label
	if (title)
	{
		// Alignment object.
		GtkWidget *alignColor = gtk_alignment_new(0.0f, 0.0f, 1.0f, 1.0f);
		gtk_alignment_set_padding(GTK_ALIGNMENT(alignColor), 0, 0, 0, 8);
		gtk_widget_show(alignColor);
		gtk_box_pack_start(GTK_BOX(container), alignColor, false, false, 0);
		
		// Label object.
		GtkWidget *lblColor = gtk_label_new(title);
		gtk_widget_show(lblColor);
		gtk_container_add(GTK_CONTAINER(alignColor), lblColor);
	}
	
	// HBox for the color buttons.
	GtkWidget *hboxButtons = gtk_hbox_new(true, 0);
	gtk_widget_show(hboxButtons);
	gtk_box_pack_start(GTK_BOX(container), hboxButtons, false, false, 0);
	
	// Color Buttons
	GtkWidget *optColorButton;
	GSList *gslColorButton = NULL;
	GdkColor tmpColor;
	
	for (int i = 0; i < numColors; i++)
	{
		if (!colors[i].name[0])
			return;
		
		optColorButton = gtk_radio_button_new(gslColorButton);
		gslColorButton = gtk_radio_button_get_group(GTK_RADIO_BUTTON(optColorButton));
		
		// Display as a regular button, not as a radio button.
		gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(optColorButton), false);
		
		// Set colors.
		gdk_color_parse(colors[i].color_normal, &tmpColor);
		gtk_widget_modify_bg(optColorButton, GTK_STATE_NORMAL, &tmpColor);
		gdk_color_parse(colors[i].color_active, &tmpColor);
		gtk_widget_modify_bg(optColorButton, GTK_STATE_ACTIVE, &tmpColor);
		gdk_color_parse(colors[i].color_prelight, &tmpColor);
		gtk_widget_modify_bg(optColorButton, GTK_STATE_PRELIGHT, &tmpColor);
		gdk_color_parse(colors[i].color_selected, &tmpColor);
		gtk_widget_modify_bg(optColorButton, GTK_STATE_SELECTED, &tmpColor);
		gdk_color_parse(colors[i].color_insensitive, &tmpColor);
		gtk_widget_modify_bg(optColorButton, GTK_STATE_INSENSITIVE, &tmpColor);
		
		gtk_widget_set_size_request(optColorButton, 16, 16);
		gtk_widget_show(optColorButton);
		gtk_box_pack_start(GTK_BOX(hboxButtons), optColorButton, true, true, 0);
		g_signal_connect(GTK_OBJECT(optColorButton), "toggled",
				 G_CALLBACK(genopt_window_callback_widget_changed), NULL);
		
		// Save the color button.
		if (colors == genopt_colors_OSD)
		{
			// OSD color button.
			optOSD_Color[buttonSet][i] = optColorButton;
		}
		else //if (colors == genopt_colors_IntroEffect)
		{
			// Intro Effect color button.
			optIntroEffectColor[i] = optColorButton;
		}
	}
}


/**
 * genopt_window_create_misc_settings_frame(): Create the miscellaneous settings frame.
 * @param container Container for the frame.
 */
static void genopt_window_create_misc_settings_frame(GtkWidget *container)
{
	// Miscellaneous settings frame.
	GtkWidget *fraMisc = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(fraMisc), GTK_SHADOW_ETCHED_IN);
	gtk_container_set_border_width(GTK_CONTAINER(fraMisc), 4);
	gtk_widget_show(fraMisc);
	gtk_box_pack_start(GTK_BOX(container), fraMisc, true, true, 0);
	
	// Label for the miscellaneous settings frame.
	GtkWidget *lblMisc = gtk_label_new("<b><i>Miscellaneous</i></b>");
	gtk_label_set_use_markup(GTK_LABEL(lblMisc), true);
	gtk_widget_show(lblMisc);
	gtk_frame_set_label_widget(GTK_FRAME(fraMisc), lblMisc);
	
	// VBox for the miscellaneous settings frame.
	GtkWidget *vboxMisc = gtk_vbox_new(false, 0);
	gtk_widget_show(vboxMisc);
	gtk_container_add(GTK_CONTAINER(fraMisc), vboxMisc);
	
	// Miscellaneous settings checkboxes.
	
	// Auto Fix Checksum
	chkMisc_AutoFixChecksum = gtk_check_button_new_with_label("Auto Fix Checksum");
	gtk_container_set_border_width(GTK_CONTAINER(chkMisc_AutoFixChecksum), 2);
	gtk_widget_show(chkMisc_AutoFixChecksum);
	gtk_box_pack_start(GTK_BOX(vboxMisc), chkMisc_AutoFixChecksum, false, false, 0);
	g_signal_connect(GTK_OBJECT(chkMisc_AutoFixChecksum), "toggled",
			 G_CALLBACK(genopt_window_callback_widget_changed), NULL);
	
	// Auto Fix Checksum
	chkMisc_AutoPause = gtk_check_button_new_with_label("Auto Pause");
	gtk_container_set_border_width(GTK_CONTAINER(chkMisc_AutoPause), 2);
	gtk_widget_show(chkMisc_AutoPause);
	gtk_box_pack_start(GTK_BOX(vboxMisc), chkMisc_AutoPause, false, false, 0);
	g_signal_connect(GTK_OBJECT(chkMisc_AutoPause), "toggled",
			 G_CALLBACK(genopt_window_callback_widget_changed), NULL);
	
	// Fast Blur
	chkMisc_FastBlur = gtk_check_button_new_with_label("Fast Blur");
	gtk_container_set_border_width(GTK_CONTAINER(chkMisc_FastBlur), 2);
	gtk_widget_show(chkMisc_FastBlur);
	gtk_box_pack_start(GTK_BOX(vboxMisc), chkMisc_FastBlur, false, false, 0);
	g_signal_connect(GTK_OBJECT(chkMisc_FastBlur), "toggled",
			 G_CALLBACK(genopt_window_callback_widget_changed), NULL);
	
	// Show SegaCD LEDs
	chkMisc_SegaCDLEDs = gtk_check_button_new_with_label("Show SegaCD LEDs");
	gtk_container_set_border_width(GTK_CONTAINER(chkMisc_SegaCDLEDs), 2);
	gtk_widget_show(chkMisc_SegaCDLEDs);
	gtk_box_pack_start(GTK_BOX(vboxMisc), chkMisc_SegaCDLEDs, false, false, 0);
	g_signal_connect(GTK_OBJECT(chkMisc_SegaCDLEDs), "toggled",
			 G_CALLBACK(genopt_window_callback_widget_changed), NULL);
	
	// Border Color Emulation
	chkMisc_BorderColorEmulation = gtk_check_button_new_with_label("Border Color Emulation");
	gtk_container_set_border_width(GTK_CONTAINER(chkMisc_BorderColorEmulation), 2);
	gtk_widget_show(chkMisc_BorderColorEmulation);
	gtk_box_pack_start(GTK_BOX(vboxMisc), chkMisc_BorderColorEmulation, false, false, 0);
	g_signal_connect(GTK_OBJECT(chkMisc_BorderColorEmulation), "toggled",
			 G_CALLBACK(genopt_window_callback_widget_changed), NULL);
	
	// Pause Tint
	chkMisc_PauseTint = gtk_check_button_new_with_label("Pause Tint");
	gtk_container_set_border_width(GTK_CONTAINER(chkMisc_PauseTint), 2);
	gtk_widget_show(chkMisc_PauseTint);
	gtk_box_pack_start(GTK_BOX(vboxMisc), chkMisc_PauseTint, false, false, 0);
	g_signal_connect(GTK_OBJECT(chkMisc_PauseTint), "toggled",
			 G_CALLBACK(genopt_window_callback_widget_changed), NULL);
	
	// VBox for intro effect.
	GtkWidget *vboxIntroEffect = gtk_vbox_new(false, 4);
	gtk_container_set_border_width(GTK_CONTAINER(vboxIntroEffect), 8);
	gtk_widget_show(vboxIntroEffect);
	gtk_container_add(GTK_CONTAINER(vboxMisc), vboxIntroEffect);
	
	// Label for intro effect.
	GtkWidget *lblIntroEffect = gtk_label_new("Intro Effect:");
	gtk_widget_show(lblIntroEffect);
	gtk_box_pack_start(GTK_BOX(vboxIntroEffect), lblIntroEffect, false, false, 0);
	
	// Dropdown for intro effect.
	cboIntroEffect = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(cboIntroEffect), "None");
	gtk_combo_box_append_text(GTK_COMBO_BOX(cboIntroEffect), "Gens Logo Effect");
	gtk_combo_box_append_text(GTK_COMBO_BOX(cboIntroEffect), "\"Crazy\" Effect");
	//gtk_combo_box_append_text(GTK_COMBO_BOX(cboIntroEffect), "Genesis TMSS"); // TODO: Broken.
	gtk_widget_show(cboIntroEffect);
	gtk_box_pack_start(GTK_BOX(vboxIntroEffect), cboIntroEffect, false, false, 0);
	g_signal_connect((gpointer)(cboIntroEffect), "changed",
			  G_CALLBACK(genopt_window_callback_widget_changed), NULL);
	
	// Spacing.
	GtkWidget *spacer = gtk_alignment_new(0, 0, 0, 0);
	gtk_widget_show(spacer);
	gtk_box_pack_start(GTK_BOX(vboxIntroEffect), spacer, false, false, 2);
	
	// Label for intro effect color
	GtkWidget *lblIntroEffectColor = gtk_label_new("Intro Effect Color:");
	gtk_widget_show(lblIntroEffectColor);
	gtk_box_pack_start(GTK_BOX(vboxIntroEffect), lblIntroEffectColor, false, false, 0);
	
	// Color Radio Buttons for intro effect color
	genopt_window_create_color_radio_buttons(vboxIntroEffect, NULL, genopt_colors_IntroEffect, 0);
}


/**
 * genopt_window_close(): Close the General Options window.
 */
void genopt_window_close(void)
{
	if (!genopt_window)
		return;
	
	// Destroy the window.
	gtk_widget_destroy(genopt_window);
	genopt_window = NULL;
}


/**
 * genopt_window_init(): Initialize the internal variables.
 */
static void genopt_window_init(void)
{
	// Get the current options.
	
	// Miscellaneous
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chkMisc_AutoFixChecksum), Auto_Fix_CS);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chkMisc_AutoPause), Auto_Pause);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chkMisc_FastBlur), Options::fastBlur());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chkMisc_SegaCDLEDs), Show_LED);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chkMisc_BorderColorEmulation), Video.borderColorEmulation);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chkMisc_PauseTint), Video.pauseTint);
	
	// FPS counter
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chkOSD_Enable[0]), vdraw_get_fps_enabled());
	
	unsigned char curFPSStyle = vdraw_get_fps_style();
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chkOSD_DoubleSized[0]), (curFPSStyle & 0x10));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chkOSD_Transparency[0]), (curFPSStyle & 0x08));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(optOSD_Color[0][(curFPSStyle & 0x06) >> 1]), true);
	
	// Message
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chkOSD_Enable[1]), vdraw_get_msg_enabled());
	
	unsigned char curMsgStyle = vdraw_get_msg_style();
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chkOSD_DoubleSized[1]), (curMsgStyle & 0x10));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chkOSD_Transparency[1]), (curMsgStyle & 0x08));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(optOSD_Color[1][(curMsgStyle & 0x06) >> 1]), true);
	
	// Intro effect.
	gtk_combo_box_set_active(GTK_COMBO_BOX(cboIntroEffect), Intro_Style);
	if (gtk_combo_box_get_active(GTK_COMBO_BOX(cboIntroEffect)) == -1)
		gtk_combo_box_set_active(GTK_COMBO_BOX(cboIntroEffect), 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(optIntroEffectColor[vdraw_get_intro_effect_color()]), true);
	
	// Disable the "Apply" button initially.
	gtk_widget_set_sensitive(btnApply, false);
}


/**
 * genopt_window_save(): Save the settings.
 */
static void genopt_window_save(void)
{
	// Miscellaneous
	Auto_Fix_CS = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chkMisc_AutoFixChecksum));
	Auto_Pause = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chkMisc_AutoPause));
	Options::setFastBlur(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chkMisc_FastBlur)));
	Show_LED = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chkMisc_SegaCDLEDs));
	Video.borderColorEmulation = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chkMisc_BorderColorEmulation));
	Video.pauseTint = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chkMisc_PauseTint));
	
	// If Auto Pause is enabled, deactivate emulation.
	// If Auto Pause is disabled, activate emulation.
	Settings.Active = !Auto_Pause;
	
	// FPS counter
	vdraw_set_fps_enabled(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chkOSD_Enable[0])));
	
	unsigned char curFPSStyle = vdraw_get_fps_style() & ~0x18;
	curFPSStyle |= (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chkOSD_DoubleSized[0])) ? 0x10 : 0x00);
	curFPSStyle |= (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chkOSD_Transparency[0])) ? 0x08 : 0x00);
	
	// FPS counter color
	for (unsigned int i = 0; i < 4; i++)
	{
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(optOSD_Color[0][i])))
		{
			curFPSStyle &= ~0x06;
			curFPSStyle |= (i << 1);
			break;
		}
	}
	
	vdraw_set_fps_style(curFPSStyle);
	
	// Message
	vdraw_set_msg_enabled(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chkOSD_Enable[1])));
	
	unsigned char curMsgStyle = vdraw_get_msg_style() & ~0x18;
	curMsgStyle |= (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chkOSD_DoubleSized[1])) ? 0x10 : 0x00);
	curMsgStyle |= (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chkOSD_Transparency[1])) ? 0x08 : 0x00);
	
	// Message color
	for (unsigned int i = 0; i < 4; i++)
	{
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(optOSD_Color[1][i])))
		{
			curMsgStyle &= ~0x06;
			curMsgStyle |= (i << 1);
			break;
		}
	}
	
	vdraw_set_msg_style(curMsgStyle);
	
	// Intro effect.
	Intro_Style = gtk_combo_box_get_active(GTK_COMBO_BOX(cboIntroEffect));
	if (Intro_Style < 0)
		Intro_Style = 0;
	
	for (unsigned int i = 0; i < 8; i++)
	{
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(optIntroEffectColor[i])))
		{
			vdraw_set_intro_effect_color((unsigned char)i);
			break;
		}
	}
		
	// Disable the "Apply" button.
	gtk_widget_set_sensitive(btnApply, FALSE);
}


/**
 * genopt_window_callback_close(): Close Window callback.
 * @param widget
 * @param event
 * @param user_data
 * @return FALSE to continue processing events; TRUE to stop processing events.
 */
static gboolean genopt_window_callback_close(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GSFT_UNUSED_PARAMETER(widget);
	GSFT_UNUSED_PARAMETER(event);
	GSFT_UNUSED_PARAMETER(user_data);
	
	genopt_window_close();
	return FALSE;
}


/**
 * genopt_window_callback_response(): Dialog Response callback.
 * @param dialog
 * @param response_id
 * @param user_data
 */
static void genopt_window_callback_response(GtkDialog *dialog, gint response_id, gpointer user_data)
{
	GSFT_UNUSED_PARAMETER(dialog);
	GSFT_UNUSED_PARAMETER(user_data);
	
	switch (response_id)
	{
		case GTK_RESPONSE_CANCEL:
			genopt_window_close();
			break;
		case GTK_RESPONSE_APPLY:
			genopt_window_save();
			break;
		case GTK_RESPONSE_OK:
			genopt_window_save();
			genopt_window_close();
			break;
		
		case GTK_RESPONSE_DELETE_EVENT:
		default:
			// Other event. Don't do anything.
			// Also, don't do anything when the dialog is deleted.
			break;
	}
}


/**
 * genopt_window_callback_widget_changed(): A widget was changed.
 */
static void genopt_window_callback_widget_changed(void)
{
	// Enable the "Apply" button.
	gtk_widget_set_sensitive(btnApply, TRUE);
}
