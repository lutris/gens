/**
 * GENS: (GTK+) General Options Window Callbacks.
 */

#ifndef GENERAL_OPTIONS_WINDOW_CALLBACKS_H
#define GENERAL_OPTIONS_WINDOW_CALLBACKS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gtk/gtk.h>
#include <gdk/gdkevents.h>

gboolean on_general_options_window_close(GtkWidget *widget, GdkEvent *event, gpointer user_data);
void on_button_go_Cancel_clicked(GtkButton *button, gpointer user_data);
void on_button_go_Apply_clicked(GtkButton *button, gpointer user_data);
void on_button_go_Save_clicked(GtkButton *button, gpointer user_data);

#ifdef __cplusplus
}
#endif

#endif
