/**
 * GENS: (GTK+) Select CD-ROM Window Callbacks.
 */

#ifndef GTK_SELECT_CDROM_WINDOW_CALLBACKS_H
#define GTK_SELECT_CDROM_WINDOW_CALLBACKS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gtk/gtk.h>
#include <gdk/gdkevents.h>
#include <gdk/gdkkeysyms.h>

gboolean on_select_cdrom_window_close(GtkWidget *widget, GdkEvent *event, gpointer user_data);
void on_button_SelCD_Cancel_clicked(GtkButton *button, gpointer user_data);
void on_button_SelCD_Apply_clicked(GtkButton *button, gpointer user_data);
void on_button_SelCD_Save_clicked(GtkButton *button, gpointer user_data);

#ifdef __cplusplus
}
#endif

#endif
