/**
 * GENS: (GTK+) Select CD-ROM Window.
 */

#ifndef GTK_SELECT_CDROM_WINDOW_H
#define GTK_SELECT_CDROM_WINDOW_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gtk/gtk.h>
#include "ui-common.h"

extern int CD_DriveSpeed[15];

GtkWidget* create_select_cdrom_window(void); 
extern GtkWidget *select_cdrom_window;

#ifdef __cplusplus
}
#endif

#endif
