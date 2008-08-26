/**
 * GENS: (GTK+) Main Window.
 */

#ifndef GTK_GENS_WINDOW_H
#define GTK_GENS_WINDOW_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gtk/gtk.h>

GtkWidget* create_gens_window(void); 
extern GtkWidget *gens_window;
extern int do_callbacks;

#ifdef GENS_DEBUG
// Debug menu items
extern GtkWidget *debugMenuItems[9];
extern GtkWidget *debugSeparators[2];
#endif

#ifdef __cplusplus
}
#endif

#endif
