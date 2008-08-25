/**
 * GENS: (GTK+) Controller Configuration Window Callbacks.
 */

#ifndef GAME_GENIE_WINDOW_CALLBACKS_H
#define GAME_GENIE_WINDOW_CALLBACKS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gtk/gtk.h>

gboolean on_controller_config_window_close(GtkWidget *widget, GdkEvent *event, gpointer user_data);
void on_button_cc_Cancel_clicked(GtkButton *button, gpointer user_data);

#ifdef __cplusplus
}
#endif

#endif
