#ifndef SCRSHOT_H
#define SCRSHOT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "emulator/g_main.hpp"

extern char ScrShot_Dir[GENS_PATH_MAX];

int Save_Shot(void);

#ifdef __cplusplus
}
#endif

#endif
