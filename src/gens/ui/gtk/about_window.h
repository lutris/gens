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

#ifdef __cplusplus
}
#endif

#endif
