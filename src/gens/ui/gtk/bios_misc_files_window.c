/**
 * GENS: (GTK+) BIOS/Misc Files Window.
 */

#include "bios_misc_files_window.h"
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

GtkWidget *bios_misc_files_window = NULL;

GtkAccelGroup *accel_group;
GtkTooltips *tooltips;


// If entry is NULL, it's a frame heading.
struct BIOSMiscFileEntry_t
{
	const char* title;
	const char* tag;
	char* entry;
};


struct BIOSMiscFileEntry_t BIOSMiscFiles[] =
{
	{"<b><i>Configure Genesis BIOS File</i></b>", "md_bios", NULL},
	{"Genesis", "md_bios", Genesis_Bios},
	{"<b><i>Configure 32X BIOS Files</i></b>", "32x_bios", NULL},
	{"MC68000", "mc68000", _32X_Genesis_Bios},
	{"Master SH2", "msh2", _32X_Master_Bios},
	{"Slave SH2", "ssh2", _32X_Slave_Bios},
	{"<b><i>Configure SegaCD BIOS Files</i></b>", "mcd_bios", NULL},
	{"USA", "mcd_bios_usa", US_CD_Bios},
	{"Europe", "mcd_bios_eur", EU_CD_Bios},
	{"Japan", "mcd_bios_jap", JA_CD_Bios},
	{"<b><i>Configure Miscellaneous Files</i></b>", "misc", NULL},
	{"CGOffline", "cgoffline", Settings.PathNames.CGOffline_Path},
	{"Manual", "manual", Settings.PathNames.Manual_Path},
	{NULL, NULL, NULL},
};


/**
 * create_bios_misc_files_window(): Create the BIOS/Misc Files Window.
 * @return BIOS/Misc Files Window.
 */
GtkWidget* create_bios_misc_files_window(void)
{
	GdkPixbuf *bios_misc_files_window_icon_pixbuf;
	GtkWidget *vbox_bmf;
	GtkWidget *frame_file = NULL, *label_frame_file = NULL, *table_frame_file = NULL;
	GtkWidget *label_file = NULL, *entry_file = NULL, *button_file = NULL;
	
	char tmp[64];
	int file = 0, table_row = 0;
	
	if (bios_misc_files_window)
	{
		// BIOS/Misc Files window is already created. Set focus.
		gtk_widget_grab_focus(bios_misc_files_window);
		return NULL;
	}
	
	// Create the BIOS/Misc Files window.
	bios_misc_files_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_widget_set_name(bios_misc_files_window, "bios_misc_files_window");
	gtk_container_set_border_width(GTK_CONTAINER(bios_misc_files_window), 5);
	gtk_window_set_title(GTK_WINDOW(bios_misc_files_window), "Configure BIOS/Misc Files");
	gtk_window_set_position(GTK_WINDOW(bios_misc_files_window), GTK_WIN_POS_CENTER);
	gtk_window_set_type_hint(GTK_WINDOW(bios_misc_files_window), GDK_WINDOW_TYPE_HINT_DIALOG);
	GLADE_HOOKUP_OBJECT_NO_REF(bios_misc_files_window, bios_misc_files_window, "bios_misc_files_window");
	
	// Load the window icon.
	bios_misc_files_window_icon_pixbuf = create_pixbuf("Gens2.ico");
	if (bios_misc_files_window_icon_pixbuf)
	{
		gtk_window_set_icon(GTK_WINDOW(bios_misc_files_window), bios_misc_files_window_icon_pixbuf);
		gdk_pixbuf_unref(bios_misc_files_window_icon_pixbuf);
	}
	
	// Create the main VBox.
	vbox_bmf = gtk_vbox_new(FALSE, 10);
	gtk_widget_set_name(vbox_bmf, "vbox_bmf");
	gtk_widget_show(vbox_bmf);
	gtk_container_add(GTK_CONTAINER(bios_misc_files_window), vbox_bmf);
	GLADE_HOOKUP_OBJECT(bios_misc_files_window, vbox_bmf, "vbox_bmf");
	
	// Create all frames. This will be fun!
	while (BIOSMiscFiles[file].title)
	{
		if (!BIOSMiscFiles[file].entry)
		{
			// No entry buffer. This is a new frame.
			sprintf(tmp, "frame_%s", BIOSMiscFiles[file].tag);
			frame_file = gtk_frame_new(NULL);
			gtk_widget_set_name(frame_file, tmp);
			gtk_container_set_border_width(GTK_CONTAINER(frame_file), 5);
			gtk_frame_set_shadow_type(GTK_FRAME(frame_file), GTK_SHADOW_NONE);
			gtk_widget_show(frame_file);
			gtk_box_pack_start(GTK_BOX(vbox_bmf), frame_file, TRUE, TRUE, 0);
			GLADE_HOOKUP_OBJECT(bios_misc_files_window, frame_file, tmp);
			
			// Add the frame label.
			sprintf(tmp, "label_frame_%s", BIOSMiscFiles[file].tag);
			label_frame_file = gtk_label_new(BIOSMiscFiles[file].title);
			gtk_widget_set_name(label_frame_file, tmp);
			gtk_label_set_use_markup(GTK_LABEL(label_frame_file), TRUE);
			gtk_widget_show(label_frame_file);
			gtk_frame_set_label_widget(GTK_FRAME(frame_file), label_frame_file);
			GLADE_HOOKUP_OBJECT(bios_misc_files_window, label_frame_file, tmp);
			
			// Add the frame table.
			sprintf(tmp, "table_frame_%s", BIOSMiscFiles[file].tag);
			table_frame_file = gtk_table_new(1, 3, FALSE);
			gtk_widget_set_name(table_frame_file, tmp);
			gtk_table_set_row_spacings(GTK_TABLE(table_frame_file), 5);
			gtk_table_set_col_spacings(GTK_TABLE(table_frame_file), 5);
			gtk_widget_show(table_frame_file);
			gtk_container_add(GTK_CONTAINER(frame_file), table_frame_file);
			GLADE_HOOKUP_OBJECT(bios_misc_files_window, table_frame_file, tmp);
			table_row = 0;
		}
		else
		{
			// File entry.
			
			// Check if the table needs to be resized.
			if (table_row > 0)
				gtk_table_resize(GTK_TABLE(table_frame_file), table_row + 1, 3);
			
			// Create the label for this file.
			sprintf(tmp, "label_%s", BIOSMiscFiles[file].tag);
			label_file = gtk_label_new(BIOSMiscFiles[file].title);
			gtk_widget_set_name(label_file, tmp);
			gtk_widget_set_size_request(label_file, 85, -1);
			gtk_misc_set_alignment(GTK_MISC(label_file), 0, 0.5);
			gtk_widget_show(label_file);
			gtk_table_attach(GTK_TABLE(table_frame_file), label_file,
					 0, 1, table_row, table_row + 1,
					 (GtkAttachOptions)(GTK_FILL),
					 (GtkAttachOptions)(0), 0, 0);
			GLADE_HOOKUP_OBJECT(bios_misc_files_window, label_file, tmp);
			
			// Create the entry for this file.
			sprintf(tmp, "entry_%s", BIOSMiscFiles[file].tag);
			entry_file = gtk_entry_new();
			gtk_widget_set_name(entry_file, tmp);
			gtk_widget_show(entry_file);
			gtk_widget_set_size_request(entry_file, 250, -1);
			gtk_table_attach(GTK_TABLE(table_frame_file), entry_file,
					 1, 2, table_row, table_row + 1,
					 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
					 (GtkAttachOptions)(0), 0, 0);
			GLADE_HOOKUP_OBJECT(bios_misc_files_window, entry_file, tmp);
			
			// Create the button for this file.
			// TODO: Use an icon?
			sprintf(tmp, "button_%s", BIOSMiscFiles[file].tag);
			button_file = gtk_button_new_with_label("Change...");
			gtk_widget_set_name(button_file, tmp);
			gtk_widget_show(button_file);
			gtk_table_attach(GTK_TABLE(table_frame_file), button_file,
					 2, 3, table_row, table_row + 1,
					 (GtkAttachOptions)(GTK_FILL),
					 (GtkAttachOptions)(0), 0, 0);
			GLADE_HOOKUP_OBJECT(bios_misc_files_window, button_file, tmp);
			
			table_row++;
		}
		file++;
	}
	
	
	gtk_widget_show_all(bios_misc_files_window);
	return bios_misc_files_window;
}
