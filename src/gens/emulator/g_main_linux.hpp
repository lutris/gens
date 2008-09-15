/**
 * Gens: Main loop. (Linux-specific code)
 */

#ifndef G_MAIN_LINUX_HPP
#define G_MAIN_LINUX_HPP

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/stat.h>
#include <stdlib.h>

void Get_Save_Path(char *buf, size_t n);
void Create_Save_Directory(const char *dir);

// TODO: Get rid of this.
void win2linux(char* str);

#ifdef __cplusplus
}
#endif

#endif
