/**
 * GENS: (GTK+) OpenGL Resolution Window Callbacks.
 */

#ifndef GTK_OPENGL_RESOLUTION_WINDOW_CALLBACKS_H
#define GTK_OPENGL_RESOLUTION_WINDOW_CALLBACKS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gtk/gtk.h>
#include <gdk/gdkevents.h>
#include <gdk/gdkkeysyms.h>

gboolean on_opengl_resolution_window_close(GtkWidget *widget, GdkEvent *event, gpointer user_data);
void on_button_GLRes_Cancel_clicked(GtkButton *button, gpointer user_data);
void on_button_GLRes_Apply_clicked(GtkButton *button, gpointer user_data);
void on_button_GLRes_Save_clicked(GtkButton *button, gpointer user_data);

#ifdef __cplusplus
}
#endif

#endif
