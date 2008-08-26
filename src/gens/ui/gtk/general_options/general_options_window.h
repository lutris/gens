/**
 * GENS: (GTK+) General Options Window.
 */

#ifndef GTK_GENERAL_OPTIONS_WINDOW_H
#define GTK_GENERAL_OPTIONS_WINDOW_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gtk/gtk.h>
#include "ui-common.h"

GtkWidget* create_general_options_window(void); 
extern GtkWidget *general_options_window;
extern const char* GO_MsgColors[13];
extern const char* GO_IntroEffectColors[25];

#ifdef __cplusplus
}
#endif

#endif
