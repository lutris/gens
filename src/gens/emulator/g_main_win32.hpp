/**
 * Gens: Main loop. (Win32-specific code)
 */

#ifndef G_MAIN_WIN32_HPP
#define G_MAIN_WIN32_HPP

#ifdef __cplusplus
extern "C" {
#endif

#define GENS_DIR_SEPARATOR_STR "\\"
#define GENS_DIR_SEPARATOR_CHR '\\'

#include <windows.h>

extern HWND Gens_hWnd;
extern HINSTANCE ghInstance;

void Get_Save_Path(char *buf, size_t n);
void Create_Save_Directory(const char *dir);

#ifdef __cplusplus
}
#endif

#endif /* G_MAIN_WIN32_HPP */
