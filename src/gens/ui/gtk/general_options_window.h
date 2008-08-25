/**
 * GENS: (GTK+) General Options Window.
 */

#ifndef GENERAL_OPTIONS_WINDOW_H
#define GENERAL_OPTIONS_WINDOW_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gtk/gtk.h>
#include "ui-common.h"

GtkWidget* create_general_options_window(void); 
extern GtkWidget *general_options_window;

#ifdef __cplusplus
}
#endif

#endif
