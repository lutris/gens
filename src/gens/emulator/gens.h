#ifndef _GENS_H
#define _GENS_H

#include "rom.h"

//#define CLOCK_NTSC 53700000			// More accurate for division round
//#define CLOCK_PAL  53200000

#define CLOCK_NTSC 53693175
#define CLOCK_PAL  53203424

//#define GENS_DEBUG

extern int Debug;
extern int Frame_Skip;
extern int Frame_Number;
extern int DAC_Improv;
extern int RMax_Level;
extern int GMax_Level;
extern int BMax_Level;
extern int Contrast_Level;
extern int Brightness_Level;
extern int Greyscale;
extern int Invert_Color;

int Round_Double(double val);
void Init_Tab(void);
void Recalculate_Palettes(void);
void Check_Country_Order(void);

#endif
