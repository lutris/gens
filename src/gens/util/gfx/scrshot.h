#ifndef SCRSHOT_H
#define SCRSHOT_H

#include "emulator/g_main.hpp"

#ifdef __cplusplus
extern "C" {
#endif

extern char ScrShot_Dir[GENS_PATH_MAX];

int Save_Shot(void);

#ifdef __cplusplus
}
#endif

#endif
