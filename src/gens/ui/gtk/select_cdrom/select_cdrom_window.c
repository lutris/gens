/**
 * GENS: (GTK+) Select CD-ROM Window.
 */


#include "select_cdrom_window.h"
#include "select_cdrom_window_callbacks.h"
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


GtkWidget *select_cdrom_window = NULL;

GtkAccelGroup *accel_group;


/**
 * create_select_cdrom_window(): Create the Select CD-ROM Window.
 * @return Select CD-ROM Window.
 */
GtkWidget* create_select_cdrom_window(void)
{
	GdkPixbuf *select_cdrom_window_icon_pixbuf;
	GtkWidget *vbox_SelCD;
	GtkWidget *hbutton_box_bottomRow;
	GtkWidget *button_SelCD_Cancel, *button_SelCD_Apply, *button_SelCD_Save;
	
	if (select_cdrom_window)
	{
		// Select CD-ROM window is already created. Set focus.
		gtk_widget_grab_focus(select_cdrom_window);
		return NULL;
	}
	
	accel_group = gtk_accel_group_new();
	
	// Create the Select CD-ROM window.
	CREATE_GTK_WINDOW(select_cdrom_window,
			  "select_cdrom_window",
			  "Select CD-ROM Drive",
			  select_cdrom_window_icon_pixbuf, "Gens2.ico");
	
	// Disable resizing.
	gtk_window_set_resizable(GTK_WINDOW(select_cdrom_window), FALSE);
	
	// Callbacks for if the window is closed.
	g_signal_connect((gpointer)select_cdrom_window, "delete_event",
			 G_CALLBACK(on_select_cdrom_window_close), NULL);
	g_signal_connect((gpointer)select_cdrom_window, "destroy_event",
			 G_CALLBACK(on_select_cdrom_window_close), NULL);
	
	// Create the main VBox.
	vbox_SelCD = gtk_vbox_new(FALSE, 5);
	gtk_widget_set_name(vbox_SelCD, "vbox_SelCD");
	gtk_widget_show(vbox_SelCD);
	gtk_container_add(GTK_CONTAINER(select_cdrom_window), vbox_SelCD);
	GLADE_HOOKUP_OBJECT(select_cdrom_window, vbox_SelCD, "vbox_SelCD");
	
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
}
