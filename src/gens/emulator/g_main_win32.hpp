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

extern HINSTANCE ghInstance;


// Difference between client window size and actual window size.
extern int Win32_dw;
extern int Win32_dh;

// Fonts
extern HFONT fntMain;
extern HFONT fntTitle;

void Get_Save_Path(char *buf, size_t n);
void Create_Save_Directory(const char *dir);

// TODO: Move this stuff to GensUI.
void Win32_centerOnGensWindow(HWND hWnd);
void Win32_clientResize(HWND hWnd, int width, int height);

int PASCAL WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow);

#ifdef __cplusplus
}
#endif

#endif /* G_MAIN_WIN32_HPP */
