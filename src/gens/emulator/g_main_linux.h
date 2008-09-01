/**
 * GENS: Main loop. (Linux-specific code)
 */

#ifndef G_MAIN_LINUX_H
#define G_MAIN_LINUX_H

#ifdef __cplusplus
extern "C" {
#endif

#include <SDL/SDL.h>

void Get_Save_Path(char *buf, size_t n);
void Create_Save_Directory(const char *dir);

// TODO: This isn't OS-independent...
void update_SDL_events(void);

#ifdef __cplusplus
}
#endif

#endif
