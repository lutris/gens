/**
 * * GENS: Miscellaneous GTK+ functions.
 */
 
#ifndef GTK_MISC_H
#define GTK_MISC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gtk/gtk.h>

GdkPixbuf *create_pixbuf(const gchar *filename);
GtkWidget *create_pixmap(GtkWidget *widget, const gchar *filename);
GtkWidget *lookup_widget (GtkWidget *widget, const gchar *widget_name);

#ifdef __cplusplus
}
#endif

#endif
