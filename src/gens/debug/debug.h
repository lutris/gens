#ifndef _DEBUG_68K_H
#define _DEBUG_68K_H

#include "gens.h"
#ifdef GENS_DEBUG

extern int adr_mem;
extern int nb_inst;
extern int pattern_pal;
extern int pattern_adr;
extern int Current_PC;

void Debug_Event (int key);
void Update_Debug_Screen (void);

#endif

#endif
