/**
 * GENS: Common UI functions. (GTK+)
 */
 
#ifndef UI_COMMON_GTK_H
#define UI_COMMON_GTK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gtk/gtk.h>

void UI_Set_Window_Title(const char* title);
void UI_Set_Window_Visibility (int visibility);

void UI_Hide_Embedded_Window(void);
void UI_Show_Embedded_Window(int w, int h);
int UI_Get_Embedded_WindowID(void);

void UI_MsgBox(const char* msg, const char* title);
int UI_OpenFile(const char* title, const char* initFile, FileFilterType filterType, char* retSelectedFile);
int UI_SaveFile(const char* title, const char* initFile, FileFilterType filterType, char* retSelectedFile);
int UI_SelectDir(const char* title, const char* initDir, char* retSelectedDir);

#ifdef __cplusplus
}
#endif

#endif
