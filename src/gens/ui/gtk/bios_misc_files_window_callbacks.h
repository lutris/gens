/**
 * GENS: (GTK+) BIOS/Misc Files Window Callbacks.
 */

#ifndef BIOS_MISC_FILES_CALLBACKS_H
#define BIOS_MISC_FILES_CALLBACKS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gtk/gtk.h>

gboolean on_bios_misc_files_window_close(GtkWidget *widget, GdkEvent *event, gpointer user_data);
void on_button_bmf_Change_clicked(GtkButton *button, gpointer user_data);
void on_button_bmf_Cancel_clicked(GtkButton *button, gpointer user_data);
void on_button_bmf_Save_clicked(GtkButton *button, gpointer user_data);

#ifdef __cplusplus
}
#endif

#endif
