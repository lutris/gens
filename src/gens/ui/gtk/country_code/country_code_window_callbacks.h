/**
 * GENS: (GTK+) Country Code Window Callbacks.
 */

#ifndef GTK_COUNTRY_CODE_WINDOW_CALLBACKS_H
#define GTK_COUNTRY_CODE_WINDOW_CALLBACKS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gtk/gtk.h>
#include <gdk/gdkevents.h>
#include <gdk/gdkkeysyms.h>

gboolean on_country_code_window_close(GtkWidget *widget, GdkEvent *event, gpointer user_data);
void on_button_Country_Cancel_clicked(GtkButton *button, gpointer user_data);
void on_button_Country_Apply_clicked(GtkButton *button, gpointer user_data);
void on_button_Country_Save_clicked(GtkButton *button, gpointer user_data);

#ifdef __cplusplus
}
#endif

#endif
