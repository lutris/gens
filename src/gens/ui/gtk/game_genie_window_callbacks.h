/**
 * GENS: (GTK+) Game Genie Window Callbacks.
 */

#ifndef GAME_GENIE_WINDOW_CALLBACKS_H
#define GAME_GENIE_WINDOW_CALLBACKS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gtk/gtk.h>
#include <gdk/gdkevents.h>
#include <gdk/gdkkeysyms.h>

void on_button_gg_addCode_clicked(GtkButton *button, gpointer user_data);
gboolean on_entry_gg_keypress(GtkWidget *widget, GdkEventKey *event, gpointer user_data);
void on_button_gg_delCode_clicked(GtkButton *button, gpointer user_data);
void on_button_gg_deactAllCodes_clicked(GtkButton *button, gpointer user_data);
void on_button_gg_cancel_clicked(GtkButton *button, gpointer user_data);
void on_button_gg_OK_clicked(GtkButton *button, gpointer user_data);

void on_treeview_gg_list_item_toggled(GtkCellRendererToggle *cell_renderer, gchar *path, gpointer user_data);

#ifdef __cplusplus
}
#endif

#endif
