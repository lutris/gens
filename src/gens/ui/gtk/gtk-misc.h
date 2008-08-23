/**
 * * GENS: Miscellaneous GTK+ functions.
 */
 
#ifndef GTK_MISC_H
#define GTK_MISC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gtk/gtk.h>

GtkWidget *lookup_widget (GtkWidget *widget, const gchar *widget_name);
void add_pixmap_directory(const gchar * directory);
GdkPixbuf *create_pixbuf(const gchar *filename);
GtkWidget *create_pixmap(GtkWidget *widget, const gchar *filename);

#ifdef __cplusplus
}
#endif

#endif
