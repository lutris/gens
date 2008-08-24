/**
 * GENS: (GTK+) Game Genie Window Callbacks.
 */

#ifndef GAME_GENIE_WINDOW_CALLBACKS_H
#define GAME_GENIE_WINDOW_CALLBACKS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gtk/gtk.h>

void on_button_gg_addCode_clicked(GtkButton *button, gpointer user_data);
void on_button_gg_delCode_clicked(GtkButton *button, gpointer user_data);
void on_button_gg_deactAllCodes_clicked(GtkButton *button, gpointer user_data);
void on_button_gg_cancel_clicked(GtkButton *button, gpointer user_data);
void on_button_gg_OK_clicked(GtkButton *button, gpointer user_data);

#ifdef __cplusplus
}
#endif

#endif
