#ifndef GENS_HPP
#define GENS_HPP

// GENS version information
#define GENS_APPNAME "Gens/GS"

// Maximum path name length.
// TODO: Use PATH_MAX instead?
#define GENS_PATH_MAX 1024

//#define CLOCK_NTSC 53700000			// More accurate for division round
//#define CLOCK_PAL  53200000

#define CLOCK_NTSC 53693175
#define CLOCK_PAL  53203424

/**
 * Z80_EXEC(): Z80 execution macro.
 * @param cyclesSubtract Cycles to subtract from Cycles_Z80.
 */
#define Z80_EXEC(cyclesSubtract)					\
{									\
	if (Z80_State == 3)						\
		z80_Exec(&M_Z80, Cycles_Z80 - (cyclesSubtract));	\
	else								\
		mdZ80_set_odo(&M_Z80, Cycles_Z80 - (cyclesSubtract));	\
}

#ifdef __cplusplus
extern "C" {
#endif

extern int Frame_Skip;
extern int Frame_Number;

void Init_Tab(void);
void Check_Country_Order(void);

void Set_Clock_Freq(const int system);

#ifdef __cplusplus
}
#endif

#endif /* GENS_HPP */
