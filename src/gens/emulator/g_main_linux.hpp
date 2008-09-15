/**
 * Gens: Main loop. (Linux-specific code)
 */

#ifndef G_MAIN_LINUX_HPP
#define G_MAIN_LINUX_HPP

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/stat.h>

void Get_Save_Path(char *buf, size_t n);
void Create_Save_Directory(const char *dir);

#ifdef __cplusplus
}
#endif

#endif
