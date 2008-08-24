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


// Macros from Glade used to store GtkWidget pointers.
#define GLADE_HOOKUP_OBJECT(component, widget, name)			\
{									\
	g_object_set_data_full(G_OBJECT(component), name,		\
			       gtk_widget_ref(widget),			\
			       (GDestroyNotify)gtk_widget_unref);	\
}


#define GLADE_HOOKUP_OBJECT_NO_REF(component, widget, name)		\
{									\
	g_object_set_data(G_OBJECT(component), name, widget);		\
};


#ifdef __cplusplus
}
#endif

#endif
