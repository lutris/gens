/**
 * GENS: (GTK+) Controller Configuration Window.
 */
 
#ifndef CONTROLLER_CONFIG_WINDOW_H
#define CONTROLLER_CONFIG_WINDOW_H

#ifdef __cplusplus
extern "C" {
#endif


#include "g_input.h"
#include <gtk/gtk.h>

GtkWidget* create_controller_config_window(void); 
extern GtkWidget *controller_config_window;
extern struct K_Def Keys_Config[8];


#ifdef __cplusplus
}
#endif

#endif
