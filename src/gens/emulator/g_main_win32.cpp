/**
 * Gens: Main loop. (Win32-specific code)
 */


#include "g_main_win32.hpp"


HWND Gens_hWnd;


/**
 * Get_Save_Path(): Get the default save path.
 * @param *buf Buffer to store the default save path in.
 */
void Get_Save_Path(char *buf, size_t n)
{
	GetCurrentDirectory(n, buf);
}


/**
 * Create_Save_Directory(): Create the default save directory.
 * @param *dir Directory name.
 */
void Create_Save_Directory(const char *dir)
{
	// Does nothing on Win32.
}
