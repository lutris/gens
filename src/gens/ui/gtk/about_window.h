/**
 * GENS: (GTK+) About Window.
 */

#ifndef ABOUT_WINDOW_H
#define ABOUT_WINDOW_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gtk/gtk.h>
#include "ui-common.h"

GtkWidget* create_about_window(void); 
extern GtkWidget *about_window;

gboolean on_about_window_close(GtkWidget *widget, GdkEvent *event, gpointer user_data);
void on_button_about_OK_clicked(GtkButton *button, gpointer user_data);

#ifdef __cplusplus
}
#endif

#endif
