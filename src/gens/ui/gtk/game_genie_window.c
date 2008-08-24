/**
 * GENS: Game Genie Window. (GTK+)
 */
 
#include "game_genie_window.h"
#include "genswindow.h"

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
#include "debug.h"

GtkWidget *game_genie_window;

GtkAccelGroup *accel_group;
GtkTooltips *tooltips;


/**
 * create_game_genie(): Create the Game Genie Window.
 * @return Game Genie Window.
 */
GtkWidget* create_game_genie_window(void)
{
	GdkPixbuf *game_genie_window_icon_pixbuf;
	GtkWidget *vbox_gg;
	GtkWidget *frame_gg;
	GtkWidget *label_gg_header;
	GtkWidget *label_gg_description;
	
	// Create the Game Genie window.
	game_genie_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_widget_set_name(game_genie_window, "game_genie_window");
	gtk_container_set_border_width(GTK_CONTAINER(game_genie_window), 5);
	gtk_window_set_title(GTK_WINDOW(game_genie_window), "Game Genie");
	gtk_window_set_position(GTK_WINDOW(game_genie_window), GTK_WIN_POS_CENTER);
	gtk_window_set_type_hint(GTK_WINDOW(game_genie_window), GDK_WINDOW_TYPE_HINT_DIALOG);
	GLADE_HOOKUP_OBJECT_NO_REF(game_genie_window, game_genie_window, "game_genie_window");
	
	// Load the window icon.
	game_genie_window_icon_pixbuf = create_pixbuf("Gens2.ico");
	if (game_genie_window_icon_pixbuf)
	{
		gtk_window_set_icon(GTK_WINDOW(game_genie_window), game_genie_window_icon_pixbuf);
		gdk_pixbuf_unref(game_genie_window_icon_pixbuf);
	}
	
	// Layout objects.
	vbox_gg = gtk_vbox_new(FALSE, 5);
	gtk_widget_set_name(vbox_gg, "vbox_gg");
	gtk_widget_show(vbox_gg);
	gtk_container_add(GTK_CONTAINER(game_genie_window), vbox_gg);
	GLADE_HOOKUP_OBJECT(game_genie_window, vbox_gg, "vbox_gg");	
	
	frame_gg = gtk_frame_new(NULL);
	gtk_widget_set_name(frame_gg, "frame_gg");
	gtk_widget_show(frame_gg);
	gtk_box_pack_start(GTK_BOX(vbox_gg), frame_gg, FALSE, TRUE, 0);
	gtk_frame_set_shadow_type(GTK_FRAME(frame_gg), GTK_SHADOW_NONE);
	
	// Header label
	label_gg_header = gtk_label_new("<b><i>Information about Game Genie / Patch codes</i></b>");
	gtk_widget_set_name(label_gg_header, "label_gg_header");
	gtk_label_set_use_markup(GTK_LABEL(label_gg_header), TRUE);
	gtk_widget_show(label_gg_header);
	gtk_frame_set_label_widget(GTK_FRAME(frame_gg), label_gg_header);
	
	// Description label
	label_gg_description = gtk_label_new(
		"Both Game Genie codes and Patch codes are supported.\n"
		"Highlight a code to activate it.\n"
		"Syntax for Game Genie codes: XXXX-YYYY\n"
		"Syntax for Patch codes: AAAAAA-DDDD (address-data)"
		);
	gtk_widget_set_name(label_gg_description, "label_gg_description");
	gtk_label_set_use_markup(GTK_LABEL(label_gg_description), TRUE);
	gtk_widget_show(label_gg_description);
	gtk_container_add(GTK_CONTAINER(frame_gg), label_gg_description);
	gtk_misc_set_alignment(GTK_MISC(label_gg_description), 0.02, 0);
	
	return game_genie_window;
}


/**
 * Open_Game_Genie(): Opens the Game Genie window.
 */
void Open_Game_Genie(void)
{
	GtkWidget *gg = create_game_genie_window();
	gtk_window_set_transient_for(GTK_WINDOW(gg), GTK_WINDOW(gens_window));
	
	// TODO: Populate the Game Genie window with the active codes.
	
	gtk_widget_show_all(gg);
}
