/**
 * GENS: Main loop. (Linux specific code)
 */

#ifndef G_MAIN_LINUX_H
#define G_MAIN_LINUX_H

#include <SDL.h>

void SetWindowText(const char *text);
void SetWindowVisibility (int visibility);

void Get_Save_Path(char *buf, size_t n);
void Create_Save_Directory(const char *dir);
int Init_OS_Graphics(void);
void End_OS_Graphics(void);

#endif
