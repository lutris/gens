/**
 * GENS: (GTK+) General Options Window.
 */


#include "general_options_window.h"
#include "general_options_window_callbacks.h"
#include "gens/gens_window.h"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>

// GENS GTK+ miscellaneous functions
#include "gtk-misc.h"

#include "gens.h"
#include "g_main.h"

// Includes with directory defines.
// TODO: Consolidate these into Settings.PathNames
#include "save.h"
#include "g_sdlsound.h"
#include "scrshot.h"
#include "ggenie.h"
#include "rom.h"


// Macro to create a frame (with label) and add it toa a box.
#define CREATE_BOX_FRAME(FrameWidget, FrameName, Box,				\
			 LabelWidget, LabelName, Caption)			\
{										\
	FrameWidget = gtk_frame_new(NULL);					\
	gtk_widget_set_name(FrameWidget, FrameName);				\
	gtk_container_set_border_width(GTK_CONTAINER(FrameWidget), 5);		\
	gtk_frame_set_shadow_type(GTK_FRAME(FrameWidget), GTK_SHADOW_NONE);	\
	gtk_widget_show(FrameWidget);						\
	gtk_box_pack_start(GTK_BOX(Box), FrameWidget, TRUE, TRUE, 0);	\
	GLADE_HOOKUP_OBJECT(general_options_window, FrameWidget, FrameName);	\
										\
	LabelWidget = gtk_label_new(Caption);					\
	gtk_widget_set_name(LabelWidget, LabelName);				\
	gtk_label_set_use_markup(GTK_LABEL(LabelWidget), TRUE);			\
	gtk_widget_show(LabelWidget);						\
	gtk_frame_set_label_widget(GTK_FRAME(FrameWidget), LabelWidget);	\
	GLADE_HOOKUP_OBJECT(general_options_window, LabelWidget, LabelName);	\
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
	GLADE_HOOKUP_OBJECT(general_options_window, TableWidget, TableName);	\
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
	GLADE_HOOKUP_OBJECT(general_options_window, CheckWidget, CheckName);	\
}


GtkWidget *general_options_window = NULL;

GtkAccelGroup *accel_group;

// Message colors.
const char* GO_MsgColors[13] =
{
	"white", "#FFFFFF", "#000000",
	"blue",  "#0000FF", "#FFFFFF",
	"green", "#00FF00", "#000000",
	"red",   "#FF0000", "#000000",
	NULL,
};

// Intro effect colors.
const char* GO_IntroEffectColors[25] =
{
	"black",  "#000000", "#FFFFFF",
	"blue",   "#0000FF", "#FFFFFF",
	"green",  "#00FF00", "#000000",
	"cyan",   "#00FFFF", "#000000",
	"red",    "#FF0000", "#000000",
	"purple", "#FF00FF", "#000000",
	"yellow", "#FFFF00", "#000000",
	"white",  "#FFFFFF", "#000000",
	NULL,
};

static void create_color_radio_buttons(const char* title,
				       const char* groupName, 
				       const char** colors,
				       const int num,
				       GtkWidget* container);


/**
 * create_general_options_window(): Create the General Options Window.
 * @return Directory Configuration Window.
 */
GtkWidget* create_general_options_window(void)
{
	GdkPixbuf *general_options_window_icon_pixbuf;
	GtkWidget *vbox_go;
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
	GtkWidget *hbutton_box_go_buttonRow;
	GtkWidget *button_go_Cancel, *button_go_Apply, *button_go_Save;
	
	if (general_options_window)
	{
		// General Options window is already created. Set focus.
		gtk_widget_grab_focus(general_options_window);
		return NULL;
	}
	
	accel_group = gtk_accel_group_new();
	
	// Create the General Options window.
	CREATE_GTK_WINDOW(general_options_window,
			  "general_options_window",
			  "General Options",
			  general_options_window_icon_pixbuf, "Gens2.ico");
	
	// Callbacks for if the window is closed.
	g_signal_connect((gpointer)general_options_window, "delete_event",
			 G_CALLBACK(on_general_options_window_close), NULL);
	g_signal_connect((gpointer)general_options_window, "destroy_event",
			 G_CALLBACK(on_general_options_window_close), NULL);
	
	// Create the main VBox.
	vbox_go = gtk_vbox_new(FALSE, 5);
	gtk_widget_set_name(vbox_go, "vbox_go");
	gtk_widget_show(vbox_go);
	gtk_container_add(GTK_CONTAINER(general_options_window), vbox_go);
	GLADE_HOOKUP_OBJECT(general_options_window, vbox_go, "vbox_go");
	
	// System frame
	CREATE_BOX_FRAME(frame_system, "frame_system", vbox_go,
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
	CREATE_BOX_FRAME(frame_fps, "frame_fps", vbox_go,
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
	gtk_table_attach(GTK_TABLE(table_fps), hbox_fps_colors, 1, 2, 1, 2,
			 (GtkAttachOptions)(GTK_FILL),
			 (GtkAttachOptions)(GTK_FILL), 0, 0);
	GLADE_HOOKUP_OBJECT(general_options_window, hbox_fps_colors, "hbox_fps_color");
	create_color_radio_buttons("Color:", "fps", GO_MsgColors, 4, hbox_fps_colors);
	
	// Message frame
	CREATE_BOX_FRAME(frame_message, "frame_message", vbox_go,
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
	gtk_table_attach(GTK_TABLE(table_message), hbox_message_colors, 1, 2, 1, 2,
			 (GtkAttachOptions)(GTK_FILL),
			 (GtkAttachOptions)(GTK_FILL), 0, 0);
	GLADE_HOOKUP_OBJECT(general_options_window, hbox_message_colors, "hbox_message_color");
	create_color_radio_buttons("Color:", "message", GO_MsgColors, 4, hbox_message_colors);
	
	// Miscellaneous frame
	CREATE_BOX_FRAME(frame_misc, "frame_misc", vbox_go,
			 label_misc, "label_misc", "<b><i>Miscellaneous</i></b>");
	
	// Intro effect colors
	hbox_misc_intro_effect_colors = gtk_hbox_new(FALSE, 5);
	gtk_widget_set_name(hbox_misc_intro_effect_colors, "hbox_misc_intro_effect_colors");
	gtk_widget_show(hbox_misc_intro_effect_colors);
	gtk_container_add(GTK_CONTAINER(frame_misc), hbox_misc_intro_effect_colors);
	GLADE_HOOKUP_OBJECT(general_options_window, 
			    hbox_misc_intro_effect_colors,
			    "hbox_misc_intro_effect_colors");
	create_color_radio_buttons("Intro Effect Color:", "misc_intro_effect",
				   GO_IntroEffectColors, 8, hbox_misc_intro_effect_colors);
	
	// HButton Box for the row of buttons on the bottom of the window
	hbutton_box_go_buttonRow = gtk_hbutton_box_new();
	gtk_widget_set_name(hbutton_box_go_buttonRow, "hbutton_box_go_buttonRow");
	gtk_button_box_set_layout(GTK_BUTTON_BOX(hbutton_box_go_buttonRow), GTK_BUTTONBOX_END);
	gtk_widget_show(hbutton_box_go_buttonRow);
	gtk_box_pack_start(GTK_BOX(vbox_go), hbutton_box_go_buttonRow, FALSE, FALSE, 0);
	GLADE_HOOKUP_OBJECT(general_options_window, hbutton_box_go_buttonRow, "hbutton_box_go_buttonRow");
	
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
	
	// Add the accel group.
	gtk_window_add_accel_group(GTK_WINDOW(general_options_window), accel_group);
	
	return general_options_window;
}


/**
 * create_color_radio_buttons(): Create color radio buttons.
 * @param title Title for this color button group.
 * @param groupName Prefix for each button's name.
 * @param colors Array of colors.
 * @param num Number of colors to use.
 * @param container Container for the radio buttons.
 */
static void create_color_radio_buttons(const char* title,
				       const char* groupName,
				       const char** colors,
				       const int num,
				       GtkWidget* container)
{
	GtkWidget *label_color, *radio_button_color;
	GSList *color_group = NULL;
	GdkColor tmpColor; int i;
	char tmp[64];
	
	// Color label
	sprintf(tmp, "label_%s_color", groupName);
	label_color = gtk_label_new(title);
	gtk_widget_set_name(label_color, tmp);
	gtk_widget_show(label_color);
	gtk_box_pack_start(GTK_BOX(container), label_color, TRUE, TRUE, 0);
	GLADE_HOOKUP_OBJECT(general_options_window, label_color, tmp);
	
	// Color Buttons
	for (i = 0; i < num; i++)
	{
		if (!colors[i * 3])
			return;
		
		sprintf(tmp, "radio_button_%s_color_%s", groupName, colors[i * 3]);
		radio_button_color = gtk_radio_button_new(color_group);
		color_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(radio_button_color));
		gtk_widget_set_name(radio_button_color, tmp);
		gdk_color_parse(colors[(i * 3) + 1], &tmpColor);
		gtk_widget_modify_bg(radio_button_color, GTK_STATE_NORMAL, &tmpColor);
		gdk_color_parse(colors[(i * 3) + 2], &tmpColor);
		gtk_widget_modify_fg(radio_button_color, GTK_STATE_NORMAL, &tmpColor);
		gtk_widget_show(radio_button_color);
		gtk_box_pack_start(GTK_BOX(container), radio_button_color, TRUE, TRUE, 0);
		GLADE_HOOKUP_OBJECT(general_options_window, radio_button_color, tmp);
	}	
}
