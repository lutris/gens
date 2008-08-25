/**
 * GENS: (GTK+) Controller Configuration Window Callbacks.
 */

#ifndef CONTROLLER_CONFIG_WINDOW_CALLBACKS_H
#define CONTROLLER_CONFIG_WINDOW_CALLBACKS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gtk/gtk.h>

gboolean on_controller_config_window_close(GtkWidget *widget, GdkEvent *event, gpointer user_data);
void on_button_cc_Cancel_clicked(GtkButton *button, gpointer user_data);
void on_button_cc_Save_clicked(GtkButton *button, gpointer user_data);
void on_button_cc_Reconfigure_clicked(GtkButton *button, gpointer user_data);
void on_check_cc_Teamplayer_clicked(GtkButton *button, gpointer user_data);

#ifdef __cplusplus
}
#endif

#endif
