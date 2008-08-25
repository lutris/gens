/**
 * GENS: (GTK+) General Options Window.
 */

#include "general_options_window.h"
#include "general_options_window_callbacks.h"
#include "gens_window.h"

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


// Macro to create a frame (with label) and add it toa a container.
#define CREATE_CONTAINER_FRAME(FrameWidget, FrameName, Container,		\
			       LabelWidget, LabelName, Caption)			\
{										\
	FrameWidget = gtk_frame_new(NULL);					\
	gtk_widget_set_name(FrameWidget, FrameName);				\
	gtk_container_set_border_width(GTK_CONTAINER(FrameWidget), 5);		\
	gtk_frame_set_shadow_type(GTK_FRAME(FrameWidget), GTK_SHADOW_NONE);	\
	gtk_widget_show(FrameWidget);						\
	gtk_container_add(GTK_CONTAINER(Container), FrameWidget);		\
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
	CREATE_CONTAINER_FRAME(frame_system, "frame_system", vbox_go,
			       label_system, "label_system", "<b><i>System</i></b>");
	
	// System table
	CREATE_CONTAINER_TABLE(table_system, "table_system", frame_system, 2, 2, 47);
	
	// Auto Fix Checksum
	CREATE_TABLE_CHECKBOX(check_system_autofixchecksum, "check_system_autofixchecksum",
			      "Auto Fix Checksum", table_system, 0, 1, 0, 1);
	
	// Auto Pause
	CREATE_TABLE_CHECKBOX(check_system_autopause, "check_system_autopause",
			      "Auto Pause", table_system, 1, 2, 0, 1);
	
	// Auto Fix Checksum
	CREATE_TABLE_CHECKBOX(check_system_fastblur, "check_system_fastblur",
			      "Fast Blur", table_system, 0, 1, 1, 2);
	
	// Show SegaCD LEDs
	CREATE_TABLE_CHECKBOX(check_system_segacd_leds, "check_system_segacd_leds",
			      "Show SegaCD LEDs", table_system, 1, 2, 1, 2);
	
	gtk_widget_show_all(general_options_window);
	return general_options_window;
}
