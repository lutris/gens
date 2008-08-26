/**
 * GENS: (GTK+) OpenGL Resolution Window.
 */

#ifndef GTK_OPENGL_RESOLUTION_WINDOW_H
#define GTK_OPENGL_RESOLUTION_WINDOW_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gtk/gtk.h>
#include "ui-common.h"

GtkWidget* create_opengl_resolution_window(void); 
extern GtkWidget *opengl_resolution_window;

#ifdef __cplusplus
}
#endif

#endif
