#ifndef GENS_IO_OLD_H
#define GENS_IO_OLD_H

#ifdef __cplusplus
extern "C" {
#endif

extern unsigned int Controller_1_State;
extern unsigned int Controller_1_COM;
extern unsigned int Controller_1_Counter;
extern unsigned int Controller_1_Delay;

extern unsigned int Controller_2_State;
extern unsigned int Controller_2_COM;
extern unsigned int Controller_2_Counter;
extern unsigned int Controller_2_Delay;

// These are only for io_old.asm compatibility.
#define CREATE_CONTROLLER_VARIABLES(player)			\
	extern unsigned int Controller_ ## player ## _Type;	\
	extern unsigned int Controller_ ## player ## _Up;	\
	extern unsigned int Controller_ ## player ## _Down;	\
	extern unsigned int Controller_ ## player ## _Left;	\
	extern unsigned int Controller_ ## player ## _Right;	\
	extern unsigned int Controller_ ## player ## _Start;	\
	extern unsigned int Controller_ ## player ## _A;	\
	extern unsigned int Controller_ ## player ## _B;	\
	extern unsigned int Controller_ ## player ## _C;	\
	extern unsigned int Controller_ ## player ## _Mode;	\
	extern unsigned int Controller_ ## player ## _X;	\
	extern unsigned int Controller_ ## player ## _Y;	\
	extern unsigned int Controller_ ## player ## _Z;

CREATE_CONTROLLER_VARIABLES(1);
CREATE_CONTROLLER_VARIABLES(1B);
CREATE_CONTROLLER_VARIABLES(1C);
CREATE_CONTROLLER_VARIABLES(1D);

CREATE_CONTROLLER_VARIABLES(2);
CREATE_CONTROLLER_VARIABLES(2B);
CREATE_CONTROLLER_VARIABLES(2C);
CREATE_CONTROLLER_VARIABLES(2D);

unsigned char RD_Controller_1_TP(void);
unsigned char RD_Controller_2_TP(void);
void Make_IO_Table(void);

#ifdef __cplusplus
}
#endif

#endif
