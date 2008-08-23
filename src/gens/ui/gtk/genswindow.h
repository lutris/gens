/**
 * GENS: Main Window. (GTK+)
 */

#ifndef GENSWINDOW_H
#define GENSWINDOW_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gtk/gtk.h>

GtkWidget* create_gens_window(void); 
extern GtkWidget *gens_window;
extern int do_callbacks;

#ifdef __cplusplus
}
#endif

#endif
