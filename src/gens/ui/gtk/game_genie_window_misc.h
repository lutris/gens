/**
 * GENS: (GTK+) Game Genie Window - Miscellaneous Functions.
 */

#ifndef GAME_GENIE_WINDOW_MISC_H
#define GAME_GENIE_WINDOW_MISC_H

#ifdef __cplusplus
extern "C" {
#endif


#include <gtk/gtk.h>


void Open_Game_Genie(void);
void GG_AddCode(GtkWidget *treeview, const char *name, const char *code, int enabled);
void GG_DelSelectedCode(void);
void GG_DeactivateAllCodes(void);
void GG_SaveCodes(void);


#ifdef __cplusplus
}
#endif

#endif
