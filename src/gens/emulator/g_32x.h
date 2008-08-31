/**
 * GENS: Sega 32X initialization and main loop code.
 */

#ifndef _GENS_32X_H
#define _GENS_32X_H

#ifdef __cplusplus
extern "C" {
#endif

#include "util/file/rom.hpp"

int Init_32X(struct Rom *MD_Rom);
void Reset_32X();
int Do_32X_VDP_Only(void);
int Do_32X_Frame_No_VDP(void);
int Do_32X_Frame(void);

#ifdef __cplusplus
}
#endif

#endif
