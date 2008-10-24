#ifndef GENS_IMAGEUTIL_HPP
#define GENS_IMAGEUTIL_HPP

// TODO: Eliminate this include.
// Move GENS_PATH_MAX somewhere else.
#include "emulator/g_main.hpp"

#ifdef __cplusplus
extern "C" {
#endif

extern char ScrShot_Dir[GENS_PATH_MAX];

int Save_Shot(void);

#ifdef __cplusplus
}
#endif

#endif /* GENS_IMAGEUTIL_HPP */
