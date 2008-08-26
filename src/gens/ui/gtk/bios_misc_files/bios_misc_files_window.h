/**
 * GENS: (GTK+) BIOS/Misc Files Window.
 */

#ifndef GTK_BIOS_MISC_FILES_WINDOW_H
#define GTK_BIOS_MISC_FILES_WINDOW_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gtk/gtk.h>
#include "ui-common.h"

// BIOS/Misc File entries
// If entry is NULL, it's a frame heading.
struct BIOSMiscFileEntry_t
{
	const char* title;
	const char* tag;
	FileFilterType filter;
	char* entry;
};

// Contains all the BIOS/Misc File entries.
extern const struct BIOSMiscFileEntry_t BIOSMiscFiles[];

GtkWidget* create_bios_misc_files_window(void); 
extern GtkWidget *bios_misc_files_window;

#ifdef __cplusplus
}
#endif

#endif
