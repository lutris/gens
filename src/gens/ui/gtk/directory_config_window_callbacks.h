/**
 * GENS: (GTK+) BIOS/Misc Files Window Callbacks.
 */

#ifndef DIRECTORY_CONFIG_WINDOW_CALLBACKS_H
#define DIRECTORY_CONFIG_WINDOW_CALLBACKS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gtk/gtk.h>
#include <gdk/gdkevents.h>

gboolean on_directory_config_window_close(GtkWidget *widget, GdkEvent *event, gpointer user_data);
void on_button_dc_Change_clicked(GtkButton *button, gpointer user_data);
void on_button_dc_Cancel_clicked(GtkButton *button, gpointer user_data);
void on_button_dc_Save_clicked(GtkButton *button, gpointer user_data);

#ifdef __cplusplus
}
#endif

#endif
