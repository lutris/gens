/**
 * GENS: (GTK+) Controller Configuration Window - Miscellaneous Functions.
 */

#ifndef GTK_CONTROLLER_CONFIG_WINDOW_MISC_H
#define GTK_CONTROLLER_CONFIG_WINDOW_MISC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gtk/gtk.h>

void Open_Controller_Config(void);
int Reconfigure_Input(int player, int padtype);
void Controller_Config_Save(void);

#ifdef __cplusplus
}
#endif

#endif
