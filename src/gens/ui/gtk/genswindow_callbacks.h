/**
 * GENS: Main Window Callbacks. (GTK+)
 */


#ifndef UI_GENSWINDOW_CALLBACKS_H
#define UI_GENSWINDOW_CALLBACKS_H

#ifdef __cplusplus
extern "C" {
#endif


#include <gtk/gtk.h>


// File menu
void on_FileMenu_OpenROM_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_FileMenu_BootCD_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_FileMenu_CloseROM_activate(GtkMenuItem *menuitem, gpointer user_data);


#ifdef __cplusplus
}
#endif

#endif
