/**
 * GENS: (GTK+) BIOS/Misc Files Window.
 */

#ifndef DIRECTORY_CONFIG_WINDOW_H
#define DIRECTORY_CONFIG_WINDOW_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gtk/gtk.h>
#include "ui-common.h"

// Directory entries
struct DirEntry_t
{
	const char* title;
	const char* tag;
	char* entry;
};

// Contains all the Directory entries.
extern const struct DirEntry_t DirEntries[];

GtkWidget* create_directory_config_window(void); 
extern GtkWidget *directory_config_window;

#ifdef __cplusplus
}
#endif

#endif
