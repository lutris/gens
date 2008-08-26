/**
 * GENS: (GTK+) Color Adjust Window Callbacks.
 */

#ifndef COLOR_ADJUST_WINDOW_CALLBACKS_H
#define COLOR_ADJUST_WINDOW_CALLBACKS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gtk/gtk.h>
#include <gdk/gdkevents.h>
#include <gdk/gdkkeysyms.h>

gboolean on_color_adjust_window_close(GtkWidget *widget, GdkEvent *event, gpointer user_data);
void on_button_ca_Cancel_clicked(GtkButton *button, gpointer user_data);
void on_button_ca_Apply_clicked(GtkButton *button, gpointer user_data);
void on_button_ca_Save_clicked(GtkButton *button, gpointer user_data);

#ifdef __cplusplus
}
#endif

#endif
