/***************************************************************************
 * Gens: (GTK+) General Options Window.                                    *
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

#include "general_options_window.hpp"
#include "gens/gens_window.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>

// TODO: Get rid of gtk-misc.h
#include "gtk-misc.h"

#include "emulator/g_main.hpp"


// Macro to create a frame (with label) and add it toa a box.
#define CREATE_BOX_FRAME(FrameWidget, FrameName, Box,				\
			 LabelWidget, LabelName, Caption)			\
{										\
	FrameWidget = gtk_frame_new(NULL);					\
	gtk_widget_set_name(FrameWidget, FrameName);				\
	gtk_container_set_border_width(GTK_CONTAINER(FrameWidget), 5);		\
	gtk_frame_set_shadow_type(GTK_FRAME(FrameWidget), GTK_SHADOW_NONE);	\
	gtk_widget_show(FrameWidget);						\
	gtk_box_pack_start(GTK_BOX(Box), FrameWidget, TRUE, TRUE, 0);		\
	g_object_set_data_full(G_OBJECT(m_Window), FrameName,			\
			       g_object_ref(FrameWidget), (GDestroyNotify)g_object_unref);	\
										\
	LabelWidget = gtk_label_new(Caption);					\
	gtk_widget_set_name(LabelWidget, LabelName);				\
	gtk_label_set_use_markup(GTK_LABEL(LabelWidget), TRUE);			\
	gtk_widget_show(LabelWidget);						\
	gtk_frame_set_label_widget(GTK_FRAME(FrameWidget), LabelWidget);	\
	g_object_set_data_full(G_OBJECT(m_Window), LabelName,			\
			       g_object_ref(LabelWidget), (GDestroyNotify)g_object_unref);	\
}


// Macro to create a table and add it to a container.
#define CREATE_CONTAINER_TABLE(TableWidget, TableName, Container,		\
			       Rows, Columns, ColSpacing)			\
{										\
	TableWidget = gtk_table_new(Rows, Columns, FALSE);			\
	gtk_widget_set_name(TableWidget, TableName);				\
	gtk_table_set_col_spacings(GTK_TABLE(TableWidget), ColSpacing);		\
	gtk_widget_show(TableWidget);						\
	gtk_container_add(GTK_CONTAINER(Container), TableWidget);		\
	g_object_set_data_full(G_OBJECT(m_Window), TableName,			\
			       g_object_ref(TableWidget), (GDestroyNotify)g_object_unref);	\
}


// Macro to create a checkbox and add it to a table.
#define CREATE_TABLE_CHECKBOX(CheckWidget, CheckName, Caption, Table,		\
			      LeftAttach, RightAttach, TopAttach, BottomAttach)	\
{										\
	CheckWidget = gtk_check_button_new_with_label(Caption);			\
	gtk_widget_set_name(CheckWidget, CheckName);				\
	gtk_container_set_border_width(GTK_CONTAINER(CheckWidget), 5);		\
	gtk_widget_show(CheckWidget);						\
	gtk_table_attach(GTK_TABLE(Table), CheckWidget,				\
			 LeftAttach, RightAttach,				\
			 TopAttach, BottomAttach,				\
			 (GtkAttachOptions)(GTK_FILL),				\
			 (GtkAttachOptions)(0), 0, 0);				\
	g_object_set_data_full(G_OBJECT(m_Window), CheckName,			\
			       g_object_ref(CheckWidget), (GDestroyNotify)g_object_unref);	\
}


// Message colors.
// Index: 0 = name; 1 = normal; 2 = active; 3 = prelight; 4 = selected; 5 = insensitive
const char* GeneralOptionsWindow::Colors_Msg[5][6] =
{
	{"white", "#FFFFFF", "#E0E0E0", "#FFFFFF", "#FFFFFF", "#C0C0C0"},
	{"blue",  "#0000FF", "#0000E0", "#8080FF", "#0000FF", "#1C1C1C"},
	{"green", "#00FF00", "#00E000", "#80FF80", "#00FF00", "#979797"},
	{"red",   "#FF0000", "#E00000", "#FF8080", "#FF0000", "#4C4C4C"},
	{NULL, NULL, NULL, NULL, NULL, NULL}
};

// Intro effect colors.
// Index: 0 = name; 1 = normal; 2 = active; 3 = prelight; 4 = selected; 5 = insensitive
const char* GeneralOptionsWindow::Colors_IntroEffect[9][6] =
{
	{"black",  "#000000", "#000000", "#808080", "#000000", "#808080"},
	{"blue",   "#0000FF", "#0000E0", "#8080FF", "#0000FF", "#202020"},
	{"green",  "#00FF00", "#00E000", "#80FF80", "#00FF00", "#989898"},
	{"cyan",   "#00FFFF", "#00E0E0", "#80FFFF", "#00FFFF", "#B3B3B3"},
	{"red",    "#FF0000", "#E00000", "#FF8080", "#FF0000", "#4C4C4C"},
	{"purple", "#FF00FF", "#E000E0", "#FF80FF", "#FF00FF", "#686868"},
	{"yellow", "#FFFF00", "#E0E000", "#FFFF80", "#FFFF00", "#E3E3E3"},
	{"white",  "#FFFFFF", "#E0E0E0", "#FFFFFF", "#FFFFFF", "#C0C0C0"},
	{NULL, NULL, NULL, NULL, NULL, NULL}
};


GeneralOptionsWindow* GeneralOptionsWindow::m_Instance = NULL;
GeneralOptionsWindow* GeneralOptionsWindow::Instance(GtkWindow *parent)
{
	if (m_Instance == NULL)
	{
		// Instance is deleted. Initialize the About window.
		m_Instance = new GeneralOptionsWindow();
	}
	else
	{
		// Instance already exists. Set focus.
		m_Instance->setFocus();
	}
	
	// Set modality of the window.
	m_Instance->setModal(parent);
	
	return m_Instance;
}


GeneralOptionsWindow::~GeneralOptionsWindow()
{
	if (m_Window)
		gtk_widget_destroy(GTK_WIDGET(m_Window));
	
	m_Instance = NULL;
}


gboolean GeneralOptionsWindow::GTK_Close(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	return reinterpret_cast<GeneralOptionsWindow*>(user_data)->close();
}


void GeneralOptionsWindow::dlgButtonPress(uint32_t button)
{
	switch (button)
	{
		case WndBase::BUTTON_SAVE:
			save();
			close();
			break;
		
		case WndBase::BUTTON_APPLY:
			save();
			break;
		
		case WndBase::BUTTON_CANCEL:
			close();
			break;
	}
}


/**
 * Window is closed.
 */
gboolean GeneralOptionsWindow::close(void)
{
	delete this;
	return FALSE;
}


/**
 * GeneralOptionsWindow: Create the General Options Window.
 */
GeneralOptionsWindow::GeneralOptionsWindow()
{
	m_Window = gtk_dialog_new();
	gtk_widget_set_name(GTK_WIDGET(m_Window), "GeneralOptionsWindow");
	gtk_container_set_border_width(GTK_CONTAINER(m_Window), 0);
	gtk_window_set_title(GTK_WINDOW(m_Window), "General Options");
	gtk_window_set_position(GTK_WINDOW(m_Window), GTK_WIN_POS_CENTER);
	gtk_window_set_resizable(GTK_WINDOW(m_Window), FALSE);
	gtk_window_set_type_hint(GTK_WINDOW(m_Window), GDK_WINDOW_TYPE_HINT_DIALOG);
	gtk_dialog_set_has_separator(GTK_DIALOG(m_Window), FALSE);
	
	// Set the window data.
	g_object_set_data(G_OBJECT(m_Window), "GeneralOptionsWindow", m_Window);
	
	// Load the Gens icon.
	GdkPixbuf *icon = create_pixbuf("Gens2.ico");
	if (icon)
	{
		gtk_window_set_icon(GTK_WINDOW(m_Window), icon);
		gdk_pixbuf_unref(icon);
	}
	
	// Callbacks for if the window is closed.
	g_signal_connect((gpointer)m_Window, "delete_event",
			  G_CALLBACK(GeneralOptionsWindow::GTK_Close), (gpointer)this);
	g_signal_connect((gpointer)m_Window, "destroy_event",
			  G_CALLBACK(GeneralOptionsWindow::GTK_Close), (gpointer)this);
	
	// Get the dialog VBox.
	GtkWidget *vboxDialog = GTK_DIALOG(m_Window)->vbox;
	gtk_widget_set_name(vboxDialog, "vboxDialog");
	gtk_widget_show(vboxDialog);
	g_object_set_data_full(G_OBJECT(m_Window), "vboxDialog",
			       g_object_ref(vboxDialog), (GDestroyNotify)g_object_unref);
	
	GtkWidget *frame_system, *label_system, *table_system;
	GtkWidget *check_system_autofixchecksum, *check_system_autopause;
	GtkWidget *check_system_fastblur, *check_system_segacd_leds;
	GtkWidget *frame_fps, *label_fps, *table_fps;
	GtkWidget *check_fps_enable, *check_fps_doublesized;
	GtkWidget *check_fps_transparency, *hbox_fps_colors;
	GtkWidget *frame_message, *label_message, *table_message;
	GtkWidget *check_message_enable, *check_message_doublesized;
	GtkWidget *check_message_transparency, *hbox_message_colors;
	GtkWidget *frame_misc, *label_misc, *hbox_misc_intro_effect_colors;
	
	// System frame
	CREATE_BOX_FRAME(frame_system, "frame_system", vboxDialog,
			 label_system, "label_system", "<b><i>System</i></b>");
	
	// System table
	CREATE_CONTAINER_TABLE(table_system, "table_system", frame_system, 2, 2, 47);
	
	// Auto Fix Checksum
	CREATE_TABLE_CHECKBOX(check_system_autofixchecksum, "check_system_autofixchecksum",
			      "Auto Fix Checksum", table_system, 0, 1, 0, 1);
	
	// Auto Pause
	CREATE_TABLE_CHECKBOX(check_system_autopause, "check_system_autopause",
			      "Auto Pause", table_system, 1, 2, 0, 1);
	
	// Fast Blur
	CREATE_TABLE_CHECKBOX(check_system_fastblur, "check_system_fastblur",
			      "Fast Blur", table_system, 0, 1, 1, 2);
	
	// Show SegaCD LEDs
	CREATE_TABLE_CHECKBOX(check_system_segacd_leds, "check_system_segacd_leds",
			      "Show SegaCD LEDs", table_system, 1, 2, 1, 2);
	
	// FPS frame
	CREATE_BOX_FRAME(frame_fps, "frame_fps", vboxDialog,
			 label_fps, "label_fps", "<b><i>FPS counter</i></b>");
	
	// FPS table
	CREATE_CONTAINER_TABLE(table_fps, "table_fps", frame_fps, 2, 2, 91);
	
	// FPS enable
	CREATE_TABLE_CHECKBOX(check_fps_enable, "check_fps_enable",
			      "Enable", table_fps, 0, 1, 0, 1);
	
	// FPS double sized
	CREATE_TABLE_CHECKBOX(check_fps_doublesized, "check_fps_doublesized",
			      "Double Sized", table_fps, 1, 2, 0, 1);
	
	// FPS transparency
	CREATE_TABLE_CHECKBOX(check_fps_transparency, "check_fps_transparency",
			      "Transparency", table_fps, 0, 1, 1, 2);
	
	// FPS colors
	hbox_fps_colors = gtk_hbox_new(FALSE, 5);
	gtk_widget_set_name(hbox_fps_colors, "hbox_fps_color");
	gtk_widget_show(hbox_fps_colors);
	gtk_table_attach(GTK_TABLE(table_fps), hbox_fps_colors, 1, 3, 1, 2,
			 (GtkAttachOptions)(GTK_FILL),
			 (GtkAttachOptions)(GTK_FILL), 0, 0);
	GLADE_HOOKUP_OBJECT(m_Window, hbox_fps_colors, "hbox_fps_color");
	createColorRadioButtons("Color:", "fps", Colors_Msg, 4, hbox_fps_colors);
	
	// Message frame
	CREATE_BOX_FRAME(frame_message, "frame_message", vboxDialog,
			 label_message, "label_message", "<b><i>Message</i></b>");
	
	// Message table
	CREATE_CONTAINER_TABLE(table_message, "table_message", frame_message, 2, 2, 91);
	
	// Message enable
	CREATE_TABLE_CHECKBOX(check_message_enable, "check_message_enable",
			      "Enable", table_message, 0, 1, 0, 1);
	
	// Message double sized
	CREATE_TABLE_CHECKBOX(check_message_doublesized, "check_message_doublesized",
			      "Double Sized", table_message, 1, 2, 0, 1);
	
	// Message transparency
	CREATE_TABLE_CHECKBOX(check_message_transparency, "check_message_transparency",
			      "Transparency", table_message, 0, 1, 1, 2);
	
	// Message colors
	hbox_message_colors = gtk_hbox_new(FALSE, 5);
	gtk_widget_set_name(hbox_message_colors, "hbox_message_color");
	gtk_widget_show(hbox_message_colors);
	gtk_table_attach(GTK_TABLE(table_message), hbox_message_colors, 1, 3, 1, 2,
			 (GtkAttachOptions)(GTK_FILL),
			 (GtkAttachOptions)(GTK_FILL), 0, 0);
	GLADE_HOOKUP_OBJECT(m_Window, hbox_message_colors, "hbox_message_color");
	createColorRadioButtons("Color:", "message", Colors_Msg, 4, hbox_message_colors);
	
	// Miscellaneous frame
	CREATE_BOX_FRAME(frame_misc, "frame_misc", vboxDialog,
			 label_misc, "label_misc", "<b><i>Miscellaneous</i></b>");
	
	// Intro effect colors
	hbox_misc_intro_effect_colors = gtk_hbox_new(FALSE, 5);
	gtk_widget_set_name(hbox_misc_intro_effect_colors, "hbox_misc_intro_effect_colors");
	gtk_widget_show(hbox_misc_intro_effect_colors);
	gtk_container_add(GTK_CONTAINER(frame_misc), hbox_misc_intro_effect_colors);
	GLADE_HOOKUP_OBJECT(m_Window, 
			    hbox_misc_intro_effect_colors,
			    "hbox_misc_intro_effect_colors");
	createColorRadioButtons("Intro Effect Color:", "misc_intro_effect",
				Colors_IntroEffect, 8, hbox_misc_intro_effect_colors);
	
	// Create an accelerator group.
	m_AccelTable = gtk_accel_group_new();
	
	// Add the OK button.
	addDialogButtons(m_Window, WndBase::BAlign_Default,
			 WndBase::BUTTON_CANCEL | WndBase::BUTTON_APPLY | WndBase::BUTTON_SAVE, 0,
			 WndBase::BUTTON_ALL);
	
	// Add the accel group to the window.
	gtk_window_add_accel_group(GTK_WINDOW(m_Window), GTK_ACCEL_GROUP(m_AccelTable));
	
	// Load settings.
	load();
	
	// Show the window.
	setVisible(true);
}


/**
 * createColorRadioButtons(): Create color radio buttons.
 * @param title Title for this color button group.
 * @param groupName Prefix for each button's name.
 * @param colors Array of colors.
 * @param num Number of colors to use.
 * @param container Container for the radio buttons.
 */
void GeneralOptionsWindow::createColorRadioButtons(const char* title,
						   const char* groupName,
						   const char* colors[][6],
						   const int num,
						   GtkWidget* container)
{
	GtkWidget *label_color, *radio_button_color;
	GSList *color_group = NULL;
	GdkColor tmpColor;
	int i, j;
	char tmp[64];
	
	// Color label
	sprintf(tmp, "label_%s_color", groupName);
	label_color = gtk_label_new(title);
	gtk_widget_set_name(label_color, tmp);
	gtk_widget_show(label_color);
	gtk_box_pack_start(GTK_BOX(container), label_color, TRUE, TRUE, 0);
	GLADE_HOOKUP_OBJECT(m_Window, label_color, tmp);
	
	// Color Buttons
	for (i = 0; i < num; i++)
	{
		if (!colors[i][0])
			return;
		
		sprintf(tmp, "radio_button_%s_color_%s", groupName, colors[i][0]);
		radio_button_color = gtk_radio_button_new(color_group);
		color_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(radio_button_color));
		gtk_widget_set_name(radio_button_color, tmp);
		
		// Display as a regular button, not as a radio button.
		gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(radio_button_color), FALSE);
		
		// Set colors.
		// GTK_STATE_NORMAL == 0
		// GTK_STATE_INSENSITIVE == 4
		for (j = GTK_STATE_NORMAL; j <= GTK_STATE_INSENSITIVE; j++)
		{
			gdk_color_parse(colors[i][j + 1], &tmpColor);
			gtk_widget_modify_bg(radio_button_color, (GtkStateType)j, &tmpColor);
		}
		
		gtk_widget_show(radio_button_color);
		gtk_box_pack_start(GTK_BOX(container), radio_button_color, TRUE, TRUE, 0);
		GLADE_HOOKUP_OBJECT(m_Window, radio_button_color, tmp);
	}
}


/**
 * load(): Load settings.
 */
void GeneralOptionsWindow::load(void)
{
	GtkWidget *check_system_autofixchecksum, *check_system_autopause;
	GtkWidget *check_system_fastblur, *check_system_segacd_leds;
	unsigned char curFPSStyle;
	GtkWidget *check_fps_enable, *check_fps_doublesized;
	GtkWidget *check_fps_transparency, *radio_button_fps_color;
	unsigned char curMsgStyle;
	GtkWidget *check_message_enable, *check_message_doublesized;
	GtkWidget *check_message_transparency, *radio_button_message_color;
	GtkWidget *radio_button_intro_effect_color;
	char tmp[64];
	
	// Get the current options.
	
	// System
	check_system_autofixchecksum = lookup_widget(m_Window, "check_system_autofixchecksum");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_system_autofixchecksum), Auto_Fix_CS);
	check_system_autopause = lookup_widget(m_Window, "check_system_autopause");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_system_autopause), Auto_Pause);
	check_system_fastblur = lookup_widget(m_Window, "check_system_fastblur");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_system_fastblur), draw->fastBlur());
	check_system_segacd_leds = lookup_widget(m_Window, "check_system_segacd_leds");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_system_segacd_leds), Show_LED);
	
	// FPS counter
	check_fps_enable = lookup_widget(m_Window, "check_fps_enable");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_fps_enable), (draw->fpsEnabled() ? 1 : 0));
	
	curFPSStyle = draw->fpsStyle();
	check_fps_doublesized = lookup_widget(m_Window, "check_fps_doublesized");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_fps_doublesized), (curFPSStyle & 0x10));
	check_fps_transparency = lookup_widget(m_Window, "check_fps_transparency");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_fps_transparency), (curFPSStyle & 0x08));
	
	// FPS counter color
	sprintf(tmp, "radio_button_fps_color_%s", Colors_Msg[(curFPSStyle & 0x06) >> 1][0]);
	radio_button_fps_color = lookup_widget(m_Window, tmp);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_button_fps_color), TRUE);
	
	// Message
	check_message_enable = lookup_widget(m_Window, "check_message_enable");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_message_enable), (draw->msgEnabled() ? 1 : 0));
	
	curMsgStyle = draw->msgStyle();
	check_message_doublesized = lookup_widget(m_Window, "check_message_doublesized");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_message_doublesized), (curMsgStyle & 0x10));
	check_message_transparency = lookup_widget(m_Window, "check_message_transparency");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_message_transparency), (curMsgStyle & 0x08));
	
	// Message color
	sprintf(tmp, "radio_button_message_color_%s", Colors_Msg[(curMsgStyle & 0x06) >> 1][0]);
	radio_button_message_color = lookup_widget(m_Window, tmp);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_button_message_color), TRUE);
	
	// Intro effect color
	sprintf(tmp, "radio_button_misc_intro_effect_color_%s",
		Colors_IntroEffect[draw->introEffectColor()][0]);
	radio_button_intro_effect_color = lookup_widget(m_Window, tmp);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_button_intro_effect_color), TRUE);
}


/**
 * save(): Save settings.
 */
void GeneralOptionsWindow::save(void)
{
	GtkWidget *check_system_autofixchecksum, *check_system_autopause;
	GtkWidget *check_system_fastblur, *check_system_segacd_leds;
	unsigned char curFPSStyle;
	GtkWidget *check_fps_enable, *check_fps_doublesized;
	GtkWidget *check_fps_transparency, *radio_button_fps_color;
	unsigned char curMsgStyle;
	GtkWidget *check_message_enable, *check_message_doublesized;
	GtkWidget *check_message_transparency, *radio_button_message_color;
	GtkWidget *radio_button_intro_effect_color;
	char tmp[64]; short i;
	
	// Save the current options.
	
	// System
	check_system_autofixchecksum = lookup_widget(m_Window, "check_system_autofixchecksum");
	Auto_Fix_CS = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_system_autofixchecksum));
	check_system_autopause = lookup_widget(m_Window, "check_system_autopause");
	Auto_Pause = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_system_autopause));
	check_system_fastblur = lookup_widget(m_Window, "check_system_fastblur");
	draw->setFastBlur(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_system_fastblur)));
	check_system_segacd_leds = lookup_widget(m_Window, "check_system_segacd_leds");
	Show_LED = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_system_segacd_leds));
	
	// FPS counter
	check_fps_enable = lookup_widget(m_Window, "check_fps_enable");
	draw->setFPSEnabled(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_fps_enable)));
	
	curFPSStyle = draw->fpsStyle() & ~0x18;
	check_fps_doublesized = lookup_widget(m_Window, "check_fps_doublesized");
	curFPSStyle |= (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_fps_doublesized)) ? 0x10 : 0x00);
	check_fps_transparency = lookup_widget(m_Window, "check_fps_transparency");
	curFPSStyle |= (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_fps_transparency)) ? 0x08 : 0x00);
	
	// FPS counter color
	for (i = 0; i < 4; i++)
	{
		if (!Colors_Msg[i][0])
			break;
		sprintf(tmp, "radio_button_fps_color_%s", Colors_Msg[i][0]);
		radio_button_fps_color = lookup_widget(m_Window, tmp);
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_button_fps_color)))
		{
			curFPSStyle &= ~0x06;
			curFPSStyle |= (i << 1);
			break;
		}
	}
	
	draw->setFPSStyle(curFPSStyle);
	
	// Message
	check_message_enable = lookup_widget(m_Window, "check_message_enable");
	draw->setMsgEnabled(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_message_enable)));
	
	curMsgStyle = draw->msgStyle() & ~0x18;
	check_message_doublesized = lookup_widget(m_Window, "check_message_doublesized");
	curMsgStyle |= (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_message_doublesized)) ? 0x10 : 0x00);
	check_message_transparency = lookup_widget(m_Window, "check_message_transparency");
	curMsgStyle |= (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_message_transparency)) ? 0x08 : 0x00);
	
	// Message color
	for (i = 0; i < 4; i++)
	{
		if (!Colors_Msg[i][0])
			break;
		sprintf(tmp, "radio_button_message_color_%s", Colors_Msg[i][0]);
		radio_button_message_color = lookup_widget(m_Window, tmp);
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_button_message_color)))
		{
			curMsgStyle &= ~0x06;
			curMsgStyle |= (i << 1);
			break;
		}
	}
	
	draw->setMsgStyle(curMsgStyle);
	
	// Intro effect color
	for (i = 0; i < 8; i++)
	{
		if (!Colors_IntroEffect[i][0])
			break;
		sprintf(tmp, "radio_button_misc_intro_effect_color_%s", Colors_IntroEffect[i][0]);
		radio_button_intro_effect_color = lookup_widget(m_Window, tmp);
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_button_intro_effect_color)))
		{
			draw->setIntroEffectColor((unsigned char)i);
			break;
		}
	}
}
