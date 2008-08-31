/**
 * GENS: Genesis (Mega Drive) initialization and main loop code.
 */

#ifndef _GENS_MD_H
#define _GENS_MD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rom.hpp"

void Detect_Country_Genesis(void);

void Init_Genesis_Bios(void);
int Init_Genesis(struct Rom *MD_Rom);
void Reset_Genesis();
int Do_VDP_Only(void);
int Do_Genesis_Frame_No_VDP(void);
int Do_Genesis_Frame(void);

#ifdef __cplusplus
}
#endif

#endif
