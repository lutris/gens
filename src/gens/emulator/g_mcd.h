/**
 * GENS: Sega CD (Mega CD) initialization and main loop code.
 */

#ifndef _GENS_MCD_H
#define _GENS_MCD_H

#include "rom.h"

char* Detect_Country_SegaCD(void);

int Init_SegaCD(char *iso_name);
int Reload_SegaCD(char *iso_name);
void Reset_SegaCD();
int Do_SegaCD_Frame_No_VDP(void);
int Do_SegaCD_Frame(void);
int Do_SegaCD_Frame_Cycle_Accurate(void);
int Do_SegaCD_Frame_No_VDP_Cycle_Accurate(void);

#endif
