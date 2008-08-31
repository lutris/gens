#ifndef GENS_IO_H
#define GENS_IO_H

#ifdef __cplusplus
extern "C" {
#endif

#define CREATE_CONTROLLER_VARIABLES_EXTERN(player)		\
	extern unsigned int Controller_ ## player ## _State;	\
	extern unsigned int Controller_ ## player ## _COM;	\
	extern unsigned int Controller_ ## player ## _Counter;	\
	extern unsigned int Controller_ ## player ## _Delay;	\
	extern unsigned int Controller_ ## player ## _Type;	\
	extern unsigned int Controller_ ## player ## _Up;	\
	extern unsigned int Controller_ ## player ## _Down;	\
	extern unsigned int Controller_ ## player ## _Left;	\
	extern unsigned int Controller_ ## player ## _Right;	\
	extern unsigned int Controller_ ## player ## _Start;	\
	extern unsigned int Controller_ ## player ## _Mode;	\
	extern unsigned int Controller_ ## player ## _A;	\
	extern unsigned int Controller_ ## player ## _B;	\
	extern unsigned int Controller_ ## player ## _C;	\
	extern unsigned int Controller_ ## player ## _X;	\
	extern unsigned int Controller_ ## player ## _Y;	\
	extern unsigned int Controller_ ## player ## _Z;	\

CREATE_CONTROLLER_VARIABLES_EXTERN(1);
CREATE_CONTROLLER_VARIABLES_EXTERN(1B);
CREATE_CONTROLLER_VARIABLES_EXTERN(1C);
CREATE_CONTROLLER_VARIABLES_EXTERN(1D);

CREATE_CONTROLLER_VARIABLES_EXTERN(2);
CREATE_CONTROLLER_VARIABLES_EXTERN(2B);
CREATE_CONTROLLER_VARIABLES_EXTERN(2C);
CREATE_CONTROLLER_VARIABLES_EXTERN(2D);

unsigned char RD_Controller_1(void);
unsigned char RD_Controller_2(void);
unsigned char WR_Controller_1(unsigned char data);
unsigned char WR_Controller_2(unsigned char data);
void Fix_Controllers(void);
void Make_IO_Table(void);

#ifdef __cplusplus
}
#endif

#endif /* GENS_IO_H */
